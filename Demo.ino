#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_HCSR04";
const char* password = "12345678";

#define TRIG 5
#define ECHO 18

WebServer server(80);

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  long distance = duration * 0.034 / 2;
  return distance;
}

void handleRoot() {
  long dist = getDistance();

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta http-equiv="refresh" content="1">
<title>HC-SR04 Monitor</title>
<style>
body { font-family: Arial; text-align: center; }
.alert { color: red; font-size: 30px; }
</style>
</head>
<body>
<h1>ESP32 + HC-SR04</h1>
<h2>Distance: )rawliteral" + String(dist) + R"rawliteral( cm</h2>
)rawliteral";

  if (dist < 20) {
    page += "<div class='alert'>⚠️ OBJECT DETECTED</div>";
  }

  page += "</body></html>";

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
