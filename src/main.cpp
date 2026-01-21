#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define SENSOR_COUNT 4
#define DIST_THRESHOLD 15   // cm
#define SENSOR_INTERVAL 200 // ms

const char* ssid = "ESP32_FR";
const char* password = "12345678";
uint16_t distThreshold = DIST_THRESHOLD;  // Global threshold ที่ปรับได้
unsigned long startTime = 0;

struct SensorData {
  uint8_t trig;
  uint8_t echo;
  uint32_t count;
  bool connected;
  bool lastState;
  uint16_t lastDistance;
};

SensorData sensors[SENSOR_COUNT] = {
  {5, 18, 0, false, false, 0},   // Machine 1 (Organic)
  {19, 21, 0, false, false, 0},  // Machine 2 (Recyclable)
  {22, 23, 0, false, false, 0},  // Machine 3 (General)
  {25, 26, 0, false, false, 0}   // Machine 4 (Hazardous)
};

uint16_t readDistance(uint8_t trig, uint8_t echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 25000);
  if (duration == 0) return 999;

  return duration * 0.034 / 2;
}

void TaskSensor(void* pv) {
  while (true) {
    for (int i = 0; i < SENSOR_COUNT; i++) {
      uint16_t d = readDistance(sensors[i].trig, sensors[i].echo);
      sensors[i].lastDistance = d;
      sensors[i].connected = (d != 999);

      bool detected = (d < DIST_THRESHOLD);

      if (detected && !sensors[i].lastState) {
        sensors[i].count++;
        sensors[i].lastState = true;
      }

      if (!detected) {
        sensors[i].lastState = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL));
  }
}

AsyncWebServer server(80);

void setupWeb() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "text/html", 
      "<html lang='th'><head><meta charset='UTF-8'><title>ระบบสแกนขยะ ESP32</title>"
      "<style>body{font-family:Arial;margin:20px;background:#f0f0f0}h2{color:#333;text-align:center}"
      "table{background:white;border-collapse:collapse;margin:20px auto;width:90%;max-width:500px}"
      "th,td{border:1px solid #ddd;padding:12px;text-align:center}th{background:#4CAF50;color:white}"
      "canvas{display:block;max-width:600px;margin:20px auto}#sys{background:white;padding:15px;margin:20px auto;border-radius:5px;width:90%;max-width:500px;text-align:center;font-weight:bold}"
      ".container{max-width:700px;margin:0 auto}</style>"
      "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body><div class='container'>"
      "<h2>ระบบนับขยะแบบเรียลไทม์</h2><div id='sys'>กำลังโหลดข้อมูล...</div>"
      "<table border='1'><tr><th>ประเภทขยะ</th><th>จำนวน</th><th>สถานะ</th></tr>"
      "<tr><td>อินทรีย์</td><td id='c0'>-</td><td id='s0'>-</td></tr>"
      "<tr><td>รีไซเคิล</td><td id='c1'>-</td><td id='s1'>-</td></tr>"
      "<tr><td>ทั่วไป</td><td id='c2'>-</td><td id='s2'>-</td></tr>"
      "<tr><td>อันตราย</td><td id='c3'>-</td><td id='s3'>-</td></tr></table>"
      "<div style='margin:20px;text-align:center'>"
      "<button onclick='resetCounter()' style='padding:10px 20px;background:#f44336;color:white;border:none;border-radius:5px;cursor:pointer'>🔄 รีเซ็ตตัวนับ</button>"
      "&nbsp;&nbsp;"
      "<button onclick='openSettings()' style='padding:10px 20px;background:#2196F3;color:white;border:none;border-radius:5px;cursor:pointer'>⚙️ ตั้งค่า</button>"
      "</div>"
      "<div id='settingsPanel' style='display:none;background:white;padding:20px;border-radius:5px;margin:20px;text-align:center'>"
      "<h3>ตั้งค่าระยะตรวจจับ</h3>"
      "<input type='number' id='thresholdInput' min='5' max='50' value='15' style='padding:8px;width:100px'>"
      "<span> cm</span>"
      "<button onclick='saveThreshold()' style='padding:8px 15px;background:#4CAF50;color:white;border:none;border-radius:5px;cursor:pointer;margin-left:10px'>บันทึก</button>"
      "<button onclick='closeSettings()' style='padding:8px 15px;background:#999;color:white;border:none;border-radius:5px;cursor:pointer;margin-left:5px'>ปิด</button>"
      "</div>"
      "<canvas id='chart'></canvas></div><script>"
      "const ctx=document.getElementById('chart');"
      "const chart=new Chart(ctx,{"
      "type:'bar',"
      "data:{labels:['อินทรีย์','รีไซเคิล','ทั่วไป','อันตราย'],datasets:[{label:'จำนวนขยะ',data:[0,0,0,0],backgroundColor:['#FF6B6B','#4ECDC4','#45B7D1','#FFA07A']}]},"
      "options:{responsive:true,plugins:{legend:{display:true}},scales:{y:{beginAtZero:true}}}"
      "});"
      "setInterval(async()=>{"
      "try{"
      "const s=await fetch('/status').then(r=>r.json());"
      "const ramMB=(s.heap/1024/1024).toFixed(2);"
      "document.getElementById('sys').innerHTML=`แรม: ${ramMB}MB | CPU: ${s.cpu}MHz | เวลา: ${s.uptime}วินาที | ผู้ใช้: ${s.clients}`;"
      "const d=await fetch('/sensors').then(r=>r.json());"
      "chart.data.datasets[0].data=[d[0].count,d[1].count,d[2].count,d[3].count];chart.update();"
      "for(let i=0;i<4;i++){"
      "document.getElementById('c'+i).innerText=d[i].count;"
      "document.getElementById('s'+i).innerText=d[i].connected?'พร้อม':'ขาด';"
      "}"
      "}catch(e){document.getElementById('sys').innerHTML='เชื่อมต่อ ESP32 ไม่ได้';}"
      "},1000);"
      "function resetCounter(){"
      "if(confirm('ต้องการรีเซ็ตตัวนับทั้งหมด?')){"
      "fetch('/reset',{method:'POST'}).then(()=>alert('รีเซ็ตสำเร็จ'));"
      "}"
      "}"
      "function openSettings(){"
      "fetch('/config').then(r=>r.json()).then(d=>{document.getElementById('thresholdInput').value=d.threshold;document.getElementById('settingsPanel').style.display='block'});"
      "}"
      "function closeSettings(){document.getElementById('settingsPanel').style.display='none';}"
      "function saveThreshold(){"
      "const val=document.getElementById('thresholdInput').value;"
      "fetch('/config',{method:'POST',body:'threshold='+val}).then(()=>alert('บันทึกสำเร็จ')).catch(e=>alert('Error: '+e));"
      "closeSettings();"
      "}"
      "</script></body></html>"
    );
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest* req) {
    StaticJsonDocument<256> doc;
    doc["heap"] = ESP.getFreeHeap();
    doc["cpu"] = ESP.getCpuFreqMHz();
    doc["uptime"] = millis() / 1000;
    doc["clients"] = WiFi.softAPgetStationNum();

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest* req) {
    StaticJsonDocument<512> doc;
    for (int i = 0; i < SENSOR_COUNT; i++) {
      doc[String(i)]["count"] = sensors[i].count;
      doc[String(i)]["connected"] = sensors[i].connected;
    }
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  server.on("/export", HTTP_GET, [](AsyncWebServerRequest* req) {
    String csv = "Sensor,Count\n";
    for (int i = 0; i < SENSOR_COUNT; i++) {
      csv += String(i) + "," + String(sensors[i].count) + "\n";
    }
    req->send(200, "text/csv", csv);
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n--- ESP32 Waste Counter Started ---");
  startTime = millis();

  for (int i = 0; i < SENSOR_COUNT; i++) {
    pinMode(sensors[i].trig, OUTPUT);
    pinMode(sensors[i].echo, INPUT);
  }

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed!");
  } else {
    Serial.println("LittleFS Mounted Successfully");
  }

  // สร้าง CSV header ถ้าไฟล์ไม่มี
  if (!LittleFS.exists("/data.csv")) {
    File file = LittleFS.open("/data.csv", "w");
    file.println("เวลา,เครื่องที่1,เครื่องที่2,เครื่องที่3,เครื่องที่4");
    file.close();
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);
  Serial.print("WiFi SSID: ");
  Serial.println(ssid);
  Serial.print("Threshold: ");
  Serial.print(distThreshold);
  Serial.println(" cm");

  setupWeb();

  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 1, NULL, 1);
  Serial.println("System Ready!");
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}