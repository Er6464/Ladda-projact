
## การต่อเซ็นเซอร์ (HC-SR04 × 4)

| เซ็นเซอร์ | TRIG       | ECHO       | หมายเหตุ     |
|-----------|------------|------------|--------------|
| Sensor 0  | GPIO 5     | GPIO 18    |              |
| Sensor 1  | GPIO 19    | GPIO 21    |              |
| Sensor 2  | GPIO 22    | GPIO 23    |              |
| Sensor 3  | GPIO 25    | GPIO 26    |              |

- VCC → 5V  
- GND → GND

## โปรโตคอลการทำงาน

1. **บูตเครื่อง**  
   → `Data_Init()` รีเซ็ตค่าทั้งหมด  
   → สร้าง Task อ่านเซ็นเซอร์  
   → เปิด WiFi AP + Web Server

2. **เมื่อ client เชื่อมต่อ WiFi**  
   → `Data_SetWiFiStatus(true)`  
   → เริ่มนับค่าจากเซ็นเซอร์จริง ๆ

3. **เมื่อ WiFi ขาดการเชื่อมต่อ**  
   → หยุดนับ (แต่ยังคงค่าเดิมไว้ใน RAM)

4. **กด Reset หรือ Power cycle**  
   → ทุกค่าเป็น 0 อีกครั้ง

## API Endpoints

| Method | Path       | คำอธิบาย                              | ตัวอย่างการเรียก                              |
|--------|------------|----------------------------------------|-----------------------------------------------|
| GET    | `/`        | หน้าเว็บ UI                            | http://192.168.4.1/                           |
| GET    | `/status`  | สถานะระบบ (heap, uptime, clients, …) | `curl http://192.168.4.1/status`              |
| GET    | `/sensors` | ข้อมูลเซ็นเซอร์ทั้ง 4 ตัว              | `curl http://192.168.4.1/sensors`             |
| POST   | `/reset`   | รีเซ็ตค่าทั้งหมดเป็น 0                 | `curl -X POST http://192.168.4.1/reset`       |
| GET    | `/config`  | ดึงค่า threshold ปัจจุบัน              | `curl http://192.168.4.1/config`              |
| POST   | `/config`  | เปลี่ยน threshold                      | `curl -X POST -d "threshold=18" http://...`   |

### ตัวอย่าง JSON จาก `/sensors`

```json
{
  "0": { "count": 7,  "connected": true  },
  "1": { "count": 0,  "connected": false },
  "2": { "count": 12, "connected": true  },
  "3": { "count": 4,  "connected": true  }
}
