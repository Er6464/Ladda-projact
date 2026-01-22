#!/bin/bash
# ====================================
# ESP32 ระบบเก็บข้อมูลเซ็นเซอร์
# Waste Monitoring System for ESP32
# ====================================

## 📋 ภาพรวมโครงการ

ระบบนี้ออกแบบมาเพื่อเก็บข้อมูลจากเซ็นเซอร์ 4 ตัวบน ESP32 Devkit V1 โดยมีลักษณะเฉพาะดังนี้:

### ✨ คุณสมบัติหลัก

1. **เก็บข้อมูลใน RAM เท่านั้น** - ไม่เก็บลงดิสก์ ข้อมูลจะหายไปเมื่อ Reset
2. **ต้องเชื่อมต่อ WiFi ก่อน** - ไม่มีการเก็บข้อมูลถ้า WiFi ไม่เชื่อมต่อ
3. **Web Server Localhost** - ดูข้อมูลแบบ Real-time ผ่านเบราว์เซอร์
4. **สำหรับเซ็นเซอร์ Ultrasonic** - ตรวจจับวัตถุขยะ

---

## 🏗️ โครงสร้างไฟล์

```
ESP32FR/
├── src/
│   ├── main.cpp        (ไฟล์หลัก - ควบคุม WiFi, Web Server, Sensors)
│   └── Data.c          (ระบบเก็บข้อมูล - จัดการเซ็นเซอร์ทั้ง 4 ตัว)
├── include/
│   └── Data.h          (Header - นิยามฟังก์ชันและ Structures)
├── platformio.ini      (การตั้งค่า PlatformIO)
└── data/
    └── index.html      (Web UI - แสดงผลแบบ Real-time)
```

---

## 🔌 โปรโตคอลการทำงาน

### 1️⃣ **ขั้นตอนการเริ่มต้น (Startup)**

```
┌─ ESP32 ตัวเองเปิดตัว
├─ Data_Init() - รีเซ็ตค่าทั้งหมดเป็น 0
├─ TaskSensor() - เริ่มตรวจจับเซ็นเซอร์ (FreeRTOS Task)
├─ WiFi AP Mode - รอการเชื่อมต่อ
└─ Web Server - เปิดพอร์ต 80 รอคำสั่ง
```

### 2️⃣ **เมื่อมีการเชื่อมต่อ WiFi**

```
ไคลเอนต์ WiFi เชื่อมต่อ
    ↓
Data_SetWiFiStatus(true)
    ↓
ระบบเริ่มเก็บข้อมูล (เก็บใน RAM)
    ↓
Web Browser สามารถเรียก API ได้
    ├─ GET /status   → ดึงสถานะระบบ
    ├─ GET /sensors  → ดึงข้อมูลเซ็นเซอร์
    └─ POST /reset   → รีเซ็ตค่า
```

### 3️⃣ **เมื่อ WiFi ตัดการเชื่อมต่อ**

```
WiFi ตัดการเชื่อมต่อ
    ↓
Data_SetWiFiStatus(false)
    ↓
ระบบหยุดเก็บข้อมูล (อ่านได้เท่านั้น)
    ↓
ค่าเซ็นเซอร์ยังถูกเก็บใน RAM แต่ไม่ทำงาน
```

### 4️⃣ **เมื่อ Reset บอร์ด**

```
กด Reset
    ↓
Data_Init() - เรียกใช้ใน setup()
    ↓
ทุกค่าเซ็นเซอร์กลับเป็น 0
    ↓
นับใหม่ตั้งแต่ 0
```

---

## 📡 API Endpoints

### GET `/` 
**ส่งคืน:** หน้า HTML - Web UI

```bash
curl http://192.168.4.1/
```

### GET `/status`
**ส่งคืน:** JSON - สถานะระบบ

```json
{
  "heap": 284752,
  "cpu": 240,
  "uptime": 125,
  "clients": 1
}
```

### GET `/sensors`
**ส่งคืน:** JSON - ข้อมูลเซ็นเซอร์ทั้ง 4 ตัว

```json
{
  "0": {"count": 5, "connected": true},
  "1": {"count": 3, "connected": false},
  "2": {"count": 8, "connected": true},
  "3": {"count": 2, "connected": true}
}
```

### POST `/reset`
**ส่งคืน:** ข้อความยืนยัน - รีเซ็ตค่าทั้งหมดเป็น 0

```bash
curl -X POST http://192.168.4.1/reset
```

### GET `/config`
**ส่งคืน:** JSON - การตั้งค่า

```json
{
  "threshold": 15
}
```

### POST `/config`
**ส่ง:** `threshold=20` - เปลี่ยนระยะตรวจจับ

```bash
curl -X POST -d "threshold=20" http://192.168.4.1/config
```

---

## 🔧 ฟังก์ชัน Data.c ที่ใช้ใหญ่

### `void Data_Init(void)`
เริ่มต้นระบบ - รีเซ็ตทั้งหมดเป็น 0

### `void Data_SetWiFiStatus(bool connected)`
ตั้งค่าสถานะ WiFi - ควบคุมการเก็บข้อมูล

### `bool Data_IsWiFiConnected(void)`
ตรวจสอบว่า WiFi เชื่อมต่อหรือไม่

### `uint32_t Data_IncrementSensor(uint8_t sensorIndex)`
เพิ่มจำนวนนับเซ็นเซอร์ (สำหรับ index 0-3)

### `uint8_t Data_GetAllSensors(SensorDataItem* buffer)`
ดึงข้อมูลทั้ง 4 เซ็นเซอร์

### `void Data_ResetAll(void)`
รีเซ็ตค่าทั้งหมดกลับเป็น 0

---

## 🚀 วิธีใช้งาน

### 1. Compile & Upload
```bash
cd c:\Users\acer\Documents\PlatformIO\Projects\ESP32FR
platformio run --target upload
```

### 2. เชื่อมต่อ WiFi
- เปิด WiFi บนโทรศัพท์/คอมพิวเตอร์
- ค้นหา SSID: `ESP32_FR`
- รหัสผ่าน: `12345678`

### 3. เปิด Web UI
```
http://192.168.4.1
```

หรือ IP ที่แสดงใน Serial Monitor

### 4. ดูข้อมูล
- ตารางแสดงจำนวนนับแต่ละเซ็นเซอร์
- กราฟแท่งแสดงเปรียบเทียบจำนวน
- สถานะหลัง "🟢" = เชื่อมต่อ, "🔴" = ตัดสัญญาณ

---

## 📊 ตัวอย่างการทำงาน

```
[Startup]
✓ Data system initialized
✓ LittleFS Mounted Successfully
📡 AP IP Address: 192.168.4.1
📡 WiFi SSID: ESP32_FR
📡 WiFi Password: 12345678
💡 Connect to WiFi first, then all sensor data will be stored!
✓ System Ready!

[WiFi Connected]
📡 WiFi Status: Connected (1 client)
✓ Data storage active

[Sensor Detected]
Sensor 0 count: 1
Sensor 0 count: 2
Sensor 0 count: 3

[Reset Button Pressed]
✓ Sensor data reset to 0
Sensor 0 count: 0
```

---

## ⚠️ ข้อสำคัญ

| ข้อ | รายละเอียด |
|-----|----------|
| **ข้อมูล** | เก็บใน RAM เท่านั้น (ไม่ Persistent) |
| **WiFi** | ต้องเชื่อมต่อเพื่อเก็บข้อมูล |
| **Reset** | ข้อมูลทั้งหมดจะลบถ้ากด Reset หรือ Power Off |
| **เซ็นเซอร์** | ต้องใช้ Ultrasonic HC-SR04 |
| **ระยะ** | DIST_THRESHOLD = 15 cm (ปรับได้) |

---

## 🔌 การต่อ Sensor

```
ESP32 Pin         HC-SR04
─────────────────────────
GPIO 5     ──────→ TRIG₁  (Sensor 0)
GPIO 18    ──────→ ECHO₁

GPIO 19    ──────→ TRIG₂  (Sensor 1)
GPIO 21    ──────→ ECHO₂

GPIO 22    ──────→ TRIG₃  (Sensor 2)
GPIO 23    ──────→ ECHO₃

GPIO 25    ──────→ TRIG₄  (Sensor 3)
GPIO 26    ──────→ ECHO₄

GND        ──────→ GND
5V         ──────→ VCC
```

---

## 🐛 Troubleshooting

### ❌ "WiFi Not Found"
- ตรวจสอบ SSID `ESP32_FR` ในรายการ WiFi ที่พร้อมใช้งาน
- ลองรีเซ็ต ESP32

### ❌ "Cannot Connect to 192.168.4.1"
- ตรวจสอบว่าเชื่อมต่อ WiFi `ESP32_FR` แล้ว
- ตรวจสอบ IP ใน Serial Monitor

### ❌ "Sensor Count Not Increasing"
- ตรวจสอบการต่อสายเซ็นเซอร์
- เปลี่ยน DIST_THRESHOLD หากวัตถุอยู่ไกลเกินไป

### ❌ "Data Not Saved After Reset"
- **นี่คือการออกแบบโดยตั้งใจ** - ข้อมูลเก็บใน RAM เท่านั้น
- หากต้องการเก็บถาวร ให้แก้ไข Data.c เพื่อใช้ LittleFS

---

## 📝 ไฟล์ที่แก้ไข

1. **Data.h** - Header ใหม่สำหรับฟังก์ชัน Data
2. **Data.c** - ระบบจัดการข้อมูลใหม่
3. **main.cpp** - เชื่อมต่อ Data.c + เพิ่ม WiFi checking + เพิ่ม /reset endpoint

---
