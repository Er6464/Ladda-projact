# ESP32 4-Sensor Point Counter System 🎯

ระบบตรวจจับวัตถุและนับคะแนน **4 ทิศทาง**  
ใช้ **ESP32 + HC-SR04 Ultrasonic Sensors (4 ตัว)**  
แสดงผลแบบ **Real-time ด้วย Python GUI**  
สื่อสารผ่าน **USB Serial (ไม่ใช้ WiFi)**

---

## 📌 Overview


### Features
- รองรับ Ultrasonic 4 ตัว (4 ทิศทาง)
- แสดงระยะทางเป็น cm
- แสดงสถานะสี
  - 🟢 ใกล้
  - 🟠 กลาง
  - 🔴 ไกล
- ระบบ **Point Counter + Debounce 1 วินาที**
- Auto-detect ESP32 COM Port
- ปุ่ม Reset คะแนน

---

## 📦 Hardware Components

| Component | Quantity | GPIO Pins |
|---------|----------|-----------|
| ESP32 Dev Module | 1 | - |
| HC-SR04 Ultrasonic | 4 | TRIG / ECHO |
| Jumper Wires | ~20 | - |
| Breadboard | 1 | - |
| USB Cable | 1 | Micro-USB |

---

## 🔌 Pin Configuration

| Sensor | TRIG | ECHO |
|------|------|------|
| Sensor 1 | GPIO 5 | GPIO 18 |
| Sensor 2 | GPIO 19 | GPIO 21 |
| Sensor 3 | GPIO 22 | GPIO 23 |
| Sensor 4 | GPIO 25 | GPIO 26 |

**Power**



---

## 🚀 Installation & Usage

### 1️⃣ Python Dependencies

```bash
pip install pyserial
🟢 Connected: COM3

📏 Distance
S1: 25cm [🟢]   S2: 89cm [🟠]
S3: 999cm [🔴]  S4: 45cm [🟢]

🎯 Points
[3] [1] [0] [2]
TOTAL: 6

[🔌 CONNECT]   [🔄 RESET]

