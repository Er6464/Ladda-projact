#define TRIG1 5
#define ECHO1 18
#define TRIG2 19
#define ECHO2 21
#define TRIG3 22
#define ECHO3 23
#define TRIG4 25
#define ECHO4 26

#define DETECT_DISTANCE 30  // cm

int points1=0, points2=0, points3=0, points4=0;
int lastD1=999, lastD2=999, lastD3=999, lastD4=999;
unsigned long lastHit=0;
const unsigned long debounce=1000;

long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);
  pinMode(TRIG4, OUTPUT); pinMode(ECHO4, INPUT);
  
  Serial.println("ESP32 Point Counter Ready!");
}

void loop() {
  // อ่านคำสั่ง RESET
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "RESET") {
      points1=points2=points3=points4=0;
      Serial.println("RESET_OK");
    }
  }

  long d1 = getDistance(TRIG1, ECHO1);
  long d2 = getDistance(TRIG2, ECHO2);
  long d3 = getDistance(TRIG3, ECHO3);
  long d4 = getDistance(TRIG4, ECHO4);

  // นับ points (ข้ามเส้น 30cm)
  if (d1 < DETECT_DISTANCE && lastD1 >= DETECT_DISTANCE && millis() - lastHit > debounce) {
    points1++; lastHit = millis();
    Serial.printf("HIT1:%d\n", points1);
  }
  if (d2 < DETECT_DISTANCE && lastD2 >= DETECT_DISTANCE && millis() - lastHit > debounce) {
    points2++; lastHit = millis();
    Serial.printf("HIT2:%d\n", points2);
  }
  if (d3 < DETECT_DISTANCE && lastD3 >= DETECT_DISTANCE && millis() - lastHit > debounce) {
    points3++; lastHit = millis();
    Serial.printf("HIT3:%d\n", points3);
  }
  if (d4 < DETECT_DISTANCE && lastD4 >= DETECT_DISTANCE && millis() - lastHit > debounce) {
    points4++; lastHit = millis();
    Serial.printf("HIT4:%d\n", points4);
  }

  // ส่งข้อมูลทุก 200ms (format: D1,D2,D3,D4,P1,P2,P3,P4,TOTAL)
  int total = points1 + points2 + points3 + points4;
  Serial.printf("DATA,%ld,%ld,%ld,%ld,%d,%d,%d,%d,%d\n", 
                d1,d2,d3,d4, points1,points2,points3,points4,total);
  
  lastD1=d1; lastD2=d2; lastD3=d3; lastD4=d4;
  delay(200);
}
