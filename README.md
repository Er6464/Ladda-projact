# ESP32 4-Sensor Point Counter System

ระบบตรวจจับวัตถุและนับคะแนนจาก 4 ทิศทาง  
พัฒนาด้วย ESP32 + Ultrasonic Sensor (HC-SR04)  
แสดงผลแบบ Real-time ผ่าน Python GUI โดยใช้ USB Serial Communication

---

## 1. วัตถุประสงค์ของระบบ

ระบบนี้ถูกออกแบบมาเพื่อ:
- ตรวจจับการเคลื่อนไหวหรือการเข้าใกล้วัตถุจากหลายทิศทาง
- นับจำนวนครั้งที่วัตถุผ่านเส้นอ้างอิง (Threshold)
- แสดงข้อมูลระยะทางและคะแนนแบบ Real-time บนคอมพิวเตอร์
- ใช้เป็นพื้นฐานสำหรับระบบนับคะแนน, เกม, interactive system หรือ demo ด้าน sensor

ระบบไม่ใช้ WiFi หรือ Bluetooth  
การสื่อสารทั้งหมดทำผ่าน USB Serial เพื่อความเสถียรและง่ายต่อการ debug

---

## 2. ภาพรวมการทำงานของระบบ (System Overview)

โครงสร้างการทำงานแบ่งเป็น 2 ส่วนหลัก

1. **Embedded Side (ESP32)**
   - อ่านค่าระยะจาก Ultrasonic Sensor 4 ตัว
   - ประมวลผลการข้าม Threshold
   - จัดการ debounce
   - ส่งข้อมูลทั้งหมดออกทาง Serial

2. **PC Side (Python GUI)**
   - ตรวจจับ COM Port อัตโนมัติ
   - รับข้อมูลจาก ESP32
   - แสดงระยะและคะแนนแบบ Real-time
   - ควบคุมคำสั่ง Reset

---

## 3. System Architecture





---

## 4. หลักการทำงาน (Working Principle)

1. ESP32 ส่งสัญญาณ Trigger ไปยัง Ultrasonic Sensor ทีละตัว
2. Sensor ส่ง Echo Pulse กลับมาตามระยะของวัตถุ
3. ESP32 วัดความกว้างของ Echo Pulse
4. คำนวณระยะทางเป็นเซนติเมตร
5. เปรียบเทียบระยะกับค่า Threshold (30 cm)
6. ถ้าเกิดการ “ข้ามเส้น” จากไกล → ใกล้ จะนับคะแนน
7. ส่งข้อมูลทั้งหมดไปยังคอมพิวเตอร์ผ่าน Serial
8. Python GUI อ่านข้อมูลและอัปเดตหน้าจอ

---

## 5. Hardware Specification

### ESP32 Dev Module
- MCU: Xtensa Dual-Core 240 MHz
- Logic Level: 3.3V
- USB-to-UART: CP210x
- Baud Rate: 115200

### HC-SR04 Ultrasonic Sensor
- Operating Voltage: 5V
- Measuring Range: 2 – 400 cm
- Accuracy: ±3 mm
- Trigger Pulse: 10 µs

---

## 6. Pin Mapping & Wiring

### Sensor 1



| Sensor | TRIG | ECHO |
|------|------|------|
| S1 | GPIO 5 | GPIO 18 |
| S2 | GPIO 19 | GPIO 21 |
| S3 | GPIO 22 | GPIO 23 |
| S4 | GPIO 25 | GPIO 26 |

> หมายเหตุ:  
> ขา ECHO ของ HC-SR04 เป็น 5V  
> ควรใช้ Voltage Divider เพื่อลดลงเป็น 3.3V เพื่อความปลอดภัยของ ESP32

---

## 7. Power Design

- ESP32 รับไฟจาก USB
- Sensors ใช้ไฟ 5V จาก VIN
- กระแสรวมประมาณ 100 mA
- ไม่แนะนำใช้สาย USB คุณภาพต่ำ

---

## 8. Firmware Design (ESP32)

### หน้าที่หลัก
- ควบคุมการ Trigger Sensor
- วัด Echo Time
- คำนวณระยะทาง
- ตรวจสอบ Threshold
- นับคะแนน
- ส่งข้อมูลออกทาง Serial

### สูตรคำนวณระยะ

---

## 9. Scoring Logic

- Threshold = 30 cm
- จะนับคะแนนเมื่อ:
  - ระยะเปลี่ยนจาก > 30 cm → ≤ 30 cm
- ถ้าวัตถุค้างอยู่ จะไม่นับซ้ำ

### Debounce
- ใช้เวลา 1 วินาทีต่อ sensor
- ป้องกันการนับจาก noise หรือมือค้าง

---

## 10. Serial Communication Protocol

### Format

| Field | Description |
|----|-----------|
| D1–D4 | ระยะทาง (cm) |
| P1–P4 | คะแนนแต่ละ sensor |
| TOTAL | คะแนนรวม |

---

## 11. Python GUI Design

### Libraries
- tkinter
- pyserial
- threading

### ฟังก์ชันหลัก
- Auto Detect ESP32 COM Port
- Serial Read แบบไม่ block UI
- Parse ข้อมูลจาก ESP32
- Update GUI แบบ Real-time

---

## 12. GUI Logic

### การแสดงสีตามระยะ
| ระยะ | สี |
|----|----|
| ≤ 30 cm | เขียว |
| 30–100 cm | ส้ม |
| > 100 cm / no object | แดง |

### ปุ่มควบคุม
- CONNECT: เชื่อมต่อ Serial
- RESET: ส่งคำสั่ง reset คะแนนไปยัง ESP32

---

## 13. Installation

### Python
```bash
pip install pyserial
