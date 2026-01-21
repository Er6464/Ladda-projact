/*
 * ESP32 Multi-Sensor Waste Counter System
 * รองรับ 4-8 เซ็นเซอร์ HC-SR04
 * 
 * CONFIGURATION:
 * - เปลี่ยน SENSOR_COUNT = 8 เพื่อรองรับ 8 เซ็นเซอร์
 * - เพิ่ม GPIO pins ใน sensors[] array
 * - เปลี่ยนชื่อเซ็นเซอร์ใน sensorNames[]
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// ===== CONFIGURATION =====
#define SENSOR_COUNT 4          // ปรับจำนวนเซ็นเซอร์ (4 หรือ 8)
#define DIST_THRESHOLD 15       // ระยะตรวจจับ (cm)
#define SENSOR_INTERVAL 200     // ช่วงการสแกน (ms)

const char* ssid = "ESP32_FA";
const char* password = "12345678";
uint16_t distThreshold = DIST_THRESHOLD;
unsigned long startTime = 0;

// ===== SENSOR NAMES (ปรับตามโครงการของคุณ) =====
const char* sensorNames[8] = {
  "เครื่องที่ 1", "เครื่องที่ 2", "เครื่องที่ 3", "เครื่องที่ 4",
  "เครื่องที่ 5", "เครื่องที่ 6", "เครื่องที่ 7", "เครื่องที่ 8"
};

// ===== GPIO CONFIGURATION =====
struct SensorData {
  uint8_t trig;
  uint8_t echo;
  uint32_t count;
  bool connected;
  bool lastState;
  uint16_t lastDistance;
};

SensorData sensors[SENSOR_COUNT] = {
  // สำหรับ 4 เซ็นเซอร์
  {5, 18, 0, false, false, 0},   // Sensor 1
  {19, 21, 0, false, false, 0},  // Sensor 2
  {22, 23, 0, false, false, 0},  // Sensor 3
  {25, 26, 0, false, false, 0},  // Sensor 4
  
  // เพิ่มเซ็นเซอร์ 5-8 ถ้า SENSOR_COUNT = 8
  // {GPIO_TRIG_5, GPIO_ECHO_5, 0, false, false, 0},
  // {GPIO_TRIG_6, GPIO_ECHO_6, 0, false, false, 0},
  // {GPIO_TRIG_7, GPIO_ECHO_7, 0, false, false, 0},
  // {GPIO_TRIG_8, GPIO_ECHO_8, 0, false, false, 0},
};

// ===== FUNCTIONS =====
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

      bool detected = (d < distThreshold);

      if (detected && !sensors[i].lastState) {
        sensors[i].count++;
        sensors[i].lastState = true;
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(sensors[i].count);
      }

      if (!detected) {
        sensors[i].lastState = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL));
  }
}

void resetData() {
  for (int i = 0; i < SENSOR_COUNT; i++) {
    sensors[i].count = 0;
  }
  LittleFS.remove("/data.csv");
  Serial.println("Data Reset!");
}

void logData() {
  File file = LittleFS.open("/data.csv", "a");
  if (!file) return;
  
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  file.print(timeStr);
  for (int i = 0; i < SENSOR_COUNT; i++) {
    file.print(",");
    file.print(sensors[i].count);
  }
  file.println();
  file.close();
  Serial.println("Data logged!");
}

// ===== WEB SERVER =====
AsyncWebServer server(80);

void setupWeb() {
  // สร้าง HTML แบบ Dynamic
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    String tableRows = "";
    for(int i = 0; i < SENSOR_COUNT; i++) {
      tableRows += "<tr><td>" + String(i+1) + ". " + String(sensorNames[i]) + "</td>"
                   "<td id='c" + String(i) + "'>-</td>"
                   "<td id='s" + String(i) + "'>-</td></tr>";
    }
    
    String chartLabels = "";
    String datasetData = "";
    String colors = "";
    const char* colorPalette[] = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#FFA07A", "#95E1D3", "#F38181", "#AA96DA", "#FCBAD3"};
    
    for(int i = 0; i < SENSOR_COUNT; i++) {
      if(i > 0) {
        chartLabels += ",";
        datasetData += ",";
        colors += ",";
      }
      chartLabels += "'" + String(i+1) + "'";
      datasetData += "0";
      colors += "'" + String(colorPalette[i]) + "'";
    }
    
    String html = 
      "<html lang='th'><head><meta charset='UTF-8'>"
      "<title>ระบบนับขยะ ESP32</title>"
      "<style>"
      "body{font-family:Arial;margin:20px;background:#f0f0f0}"
      "h2{color:#333;text-align:center}"
      "table{background:white;border-collapse:collapse;margin:20px auto;width:95%}"
      "th,td{border:1px solid #ddd;padding:10px;text-align:center}"
      "th{background:#4CAF50;color:white}"
      "canvas{display:block;max-width:800px;margin:20px auto}"
      "#sys{background:white;padding:15px;margin:20px auto;border-radius:5px;width:95%;text-align:center;font-weight:bold}"
      ".container{max-width:1000px;margin:0 auto}"
      ".button-group{margin:20px;text-align:center}"
      "button{padding:10px 20px;margin:5px;border:none;border-radius:5px;cursor:pointer;color:white;font-weight:bold}"
      ".btn-reset{background:#f44336}.btn-settings{background:#2196F3}.btn-save{background:#4CAF50}.btn-close{background:#999}"
      "#settingsPanel{display:none;background:white;padding:20px;border-radius:5px;margin:20px;text-align:center}"
      "input[type=number]{padding:8px;width:80px;font-size:16px}"
      "</style>"
      "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
      "</head><body><div class='container'>"
      "<h2>🗑️ ระบบนับขยะแบบเรียลไทม์</h2>"
      "<div id='sys'>กำลังโหลด...</div>"
      "<p style='text-align:center;color:#666'>รองรับ " + String(SENSOR_COUNT) + " เซ็นเซอร์</p>"
      "<table border='1'>"
      "<tr><th>เซ็นเซอร์</th><th>จำนวน</th><th>สถานะ</th></tr>"
      + tableRows +
      "</table>"
      "<div class='button-group'>"
      "<button class='btn-reset' onclick='resetCounter()'>🔄 รีเซ็ต</button>"
      "<button class='btn-settings' onclick='openSettings()'>⚙️ ตั้งค่า</button>"
      "<button class='btn-save' onclick='saveData()'>💾 บันทึก</button>"
      "</div>"
      "<div id='settingsPanel'>"
      "<h3>⚙️ ตั้งค่า</h3>"
      "<label>ระยะตรวจจับ (cm):</label><br>"
      "<input type='number' id='thresholdInput' min='5' max='50' value='15'>"
      "<button class='btn-settings' onclick='saveThreshold()'>บันทึก</button>"
      "<button class='btn-close' onclick='closeSettings()'>ปิด</button>"
      "</div>"
      "<canvas id='chart'></canvas>"
      "</div>"
      "<script>"
      "const ctx=document.getElementById('chart');"
      "const chart=new Chart(ctx,{"
      "type:'bar',"
      "data:{labels:[" + chartLabels + "],datasets:[{label:'จำนวน (ชิ้น)',data:[" + datasetData + "],backgroundColor:[" + colors + "]}]},"
      "options:{responsive:true,plugins:{legend:{display:true}},scales:{y:{beginAtZero:true,ticks:{stepSize:1}}}}"
      "});"
      "setInterval(async()=>{try{"
      "const s=await fetch('/status').then(r=>r.json());"
      "const ramMB=(s.heap/1024/1024).toFixed(2);"
      "document.getElementById('sys').innerHTML=`📊 RAM: ${ramMB}MB | 🔧 CPU: ${s.cpu}MHz | ⏱️ เวลา: ${s.uptime}วินาที | 👥 ผู้ใช้: ${s.clients}`;"
      "const d=await fetch('/sensors').then(r=>r.json());"
      "let data=[];"
      "for(let i=0;i<" + String(SENSOR_COUNT) + ";i++){"
      "data.push(d[i].count);"
      "document.getElementById('c'+i).innerText=d[i].count;"
      "document.getElementById('s'+i).innerText=d[i].connected?'✅ พร้อม':'❌ ขาด';"
      "}"
      "chart.data.datasets[0].data=data;"
      "chart.update();"
      "}catch(e){document.getElementById('sys').innerHTML='❌ เชื่อมต่อ ESP32 ไม่ได้';}},1000);"
      "function resetCounter(){if(confirm('ต้องการรีเซ็ตตัวนับทั้งหมด?')){fetch('/reset',{method:'POST'}).then(()=>{alert('✅ รีเซ็ตสำเร็จ');location.reload();});} }"
      "function openSettings(){fetch('/config').then(r=>r.json()).then(d=>{document.getElementById('thresholdInput').value=d.threshold;document.getElementById('settingsPanel').style.display='block'});}"
      "function closeSettings(){document.getElementById('settingsPanel').style.display='none';}"
      "function saveThreshold(){const val=document.getElementById('thresholdInput').value;fetch('/config',{method:'POST',body:'threshold='+val}).then(()=>{alert('✅ บันทึกสำเร็จ');closeSettings();}).catch(e=>alert('❌ Error: '+e));}"
      "function saveData(){fetch('/save',{method:'POST'}).then(()=>alert('✅ บันทึกข้อมูลสำเร็จ'));}"
      "</script></body></html>";
    
    req->send(200, "text/html", html);
  });

  // API: System Status
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

  // API: Sensor Data
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest* req) {
    DynamicJsonDocument doc(512 + SENSOR_COUNT * 50);
    for (int i = 0; i < SENSOR_COUNT; i++) {
      doc[String(i)]["count"] = sensors[i].count;
      doc[String(i)]["connected"] = sensors[i].connected;
      doc[String(i)]["distance"] = sensors[i].lastDistance;
      doc[String(i)]["name"] = sensorNames[i];
    }
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // API: Configuration
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest* req) {
    StaticJsonDocument<512> doc;
    doc["threshold"] = distThreshold;
    doc["sensorCount"] = SENSOR_COUNT;
    doc["sensorInterval"] = SENSOR_INTERVAL;
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  server.on("/config", HTTP_POST, [](AsyncWebServerRequest* req) {
    if (req->hasParam("threshold", true)) {
      distThreshold = atoi(req->getParam("threshold", true)->value().c_str());
      StaticJsonDocument<128> doc;
      doc["status"] = "updated";
      doc["threshold"] = distThreshold;
      String out;
      serializeJson(doc, out);
      req->send(200, "application/json", out);
      Serial.print("Threshold updated to: ");
      Serial.println(distThreshold);
    } else {
      req->send(400, "application/json", "{\"error\":\"missing param\"}");
    }
  });

  // API: Reset
  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest* req) {
    resetData();
    req->send(200, "application/json", "{\"status\":\"reset\"}");
  });

  // API: Save Data
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest* req) {
    logData();
    req->send(200, "application/json", "{\"status\":\"saved\"}");
  });

  // API: Export CSV
  server.on("/export", HTTP_GET, [](AsyncWebServerRequest* req) {
    String csv = "เวลา";
    for (int i = 0; i < SENSOR_COUNT; i++) {
      csv += "," + String(sensorNames[i]);
    }
    csv += "\n";
    
    if (LittleFS.exists("/data.csv")) {
      File file = LittleFS.open("/data.csv", "r");
      while (file.available()) {
        csv += file.readStringUntil('\n') + "\n";
      }
      file.close();
    }
    req->send(200, "text/csv", csv);
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
  Serial.println("Web server started!");
}

// ===== SETUP & LOOP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n========================================");
  Serial.println("ESP32 Multi-Sensor Waste Counter");
  Serial.print("Sensors: ");
  Serial.println(SENSOR_COUNT);
  Serial.println("========================================");
  
  startTime = millis();

  // Initialize pins
  for (int i = 0; i < SENSOR_COUNT; i++) {
    pinMode(sensors[i].trig, OUTPUT);
    pinMode(sensors[i].echo, INPUT);
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" (");
    Serial.print(sensorNames[i]);
    Serial.print(") - Trig: GPIO");
    Serial.print(sensors[i].trig);
    Serial.print(", Echo: GPIO");
    Serial.println(sensors[i].echo);
  }

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed!");
  } else {
    Serial.println("LittleFS Mounted OK");
    if (!LittleFS.exists("/data.csv")) {
      File file = LittleFS.open("/data.csv", "w");
      file.print("เวลา");
      for (int i = 0; i < SENSOR_COUNT; i++) {
        file.print(",");
        file.print(sensorNames[i]);
      }
      file.println();
      file.close();
    }
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("\nAP IP: ");
  Serial.println(IP);
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Threshold: ");
  Serial.print(distThreshold);
  Serial.println(" cm");

  setupWeb();

  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 1, NULL, 1);
  Serial.println("✅ System Ready!\n");
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
