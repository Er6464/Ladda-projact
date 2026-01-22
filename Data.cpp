#include <Arduino.h>
#include "Data.h"
#include <stddef.h>
#include <string.h>

// ตัวแปรเก็บข้อมูลเซ็นเซอร์ทั้ง 4 ตัว (เก็บเฉพาะใน RAM ไม่ต่อเนื่อง)
static SensorDataItem sensorData[TOTAL_SENSORS] = {
    {0, false, 0},  // Sensor 0 - Organic
    {0, false, 0},  // Sensor 1 - Recycle
    {0, false, 0},  // Sensor 2 - General
    {0, false, 0}   // Sensor 3 - Hazard
};

// ตัวแปรสถานะระบบ
static SystemStatus systemStatus = {
    .wifiConnected = false,
    .lastUpdateTime = 0
};

/**
 * @brief เริ่มต้นระบบเก็บข้อมูล
 * รีเซ็ตค่าทั้งหมดเป็น 0
 */
void Data_Init(void) {
    for (int i = 0; i < TOTAL_SENSORS; i++) {
        sensorData[i].count = 0;
        sensorData[i].connected = false;
        sensorData[i].lastDistance = 0;
    }
    systemStatus.wifiConnected = false;
    systemStatus.lastUpdateTime = 0;
}

/**
 * @brief ตั้งค่าสถานะ WiFi
 * @param connected true = เชื่อมต่อ, false = ไม่เชื่อมต่อ
 */
void Data_SetWiFiStatus(bool connected) {
    systemStatus.wifiConnected = connected;
    systemStatus.lastUpdateTime = millis();
    
    // ถ้า WiFi ขาด ให้หยุดเก็บข้อมูล
    if (!connected) {
        for (int i = 0; i < TOTAL_SENSORS; i++) {
            sensorData[i].connected = false;
        }
    }
}

/**
 * @brief ตรวจสอบการเชื่อมต่อ WiFi
 * @return true = เชื่อมต่อแล้ว, false = ยังไม่เชื่อมต่อ
 */
bool Data_IsWiFiConnected(void) {
    return systemStatus.wifiConnected;
}

/**
 * @brief ปรับปรุงข้อมูลเซ็นเซอร์
 * @param sensorIndex หมายเลขเซ็นเซอร์ (0-3)
 * @param count จำนวนสินค้า
 * @param connected สถานะการเชื่อมต่อ
 * @param distance ระยะห่าง
 * 
 * @return true = สำเร็จ, false = ล้มเหลว
 */
bool Data_UpdateSensor(uint8_t sensorIndex, uint32_t count, bool connected, uint16_t distance) {
    // ตรวจสอบ WiFi ต้องเชื่อมต่อก่อนเก็บข้อมูล
    if (!systemStatus.wifiConnected) {
        return false;
    }

    // ตรวจสอบ index ว่าอยู่ในช่วง
    if (sensorIndex >= TOTAL_SENSORS) {
        return false;
    }

    // อัปเดตข้อมูลเฉพาะเมื่อ WiFi เชื่อมต่อ
    sensorData[sensorIndex].count = count;
    sensorData[sensorIndex].connected = connected;
    sensorData[sensorIndex].lastDistance = distance;

    return true;
}

/**
 * @brief เพิ่มจำนวนนับสำหรับเซ็นเซอร์
 * @param sensorIndex หมายเลขเซ็นเซอร์ (0-3)
 * 
 * @return ค่านับใหม่ หรือ 0xFFFFFFFF ถ้าเกิดข้อผิดพลาด
 */
uint32_t Data_IncrementSensor(uint8_t sensorIndex) {
    // ตรวจสอบ WiFi ต้องเชื่อมต่อก่อน
    if (!systemStatus.wifiConnected) {
        return 0xFFFFFFFF;
    }

    // ตรวจสอบ index
    if (sensorIndex >= TOTAL_SENSORS) {
        return 0xFFFFFFFF;
    }

    // เพิ่มค่า
    sensorData[sensorIndex].count++;
    return sensorData[sensorIndex].count;
}

/**
 * @brief ดึงข้อมูลเซ็นเซอร์ทั้งหมด
 * @param buffer ที่เก็บข้อมูล (ต้องเพียงพอสำหรับ TOTAL_SENSORS)
 * @return จำนวนเซ็นเซอร์ที่ส่งกลับ
 */
uint8_t Data_GetAllSensors(SensorDataItem* buffer) {
    if (buffer == NULL) {
        return 0;
    }

    for (int i = 0; i < TOTAL_SENSORS; i++) {
        buffer[i].count = sensorData[i].count;
        buffer[i].connected = sensorData[i].connected;
        buffer[i].lastDistance = sensorData[i].lastDistance;
    }

    return TOTAL_SENSORS;
}

/**
 * @brief ดึงข้อมูลเซ็นเซอร์ตัวเดียว
 * @param sensorIndex หมายเลขเซ็นเซอร์ (0-3)
 * @param sensor ตัวแปรสำหรับเก็บข้อมูล
 * 
 * @return true = สำเร็จ, false = ล้มเหลว
 */
bool Data_GetSensor(uint8_t sensorIndex, SensorDataItem* sensor) {
    if (sensor == NULL || sensorIndex >= TOTAL_SENSORS) {
        return false;
    }

    sensor->count = sensorData[sensorIndex].count;
    sensor->connected = sensorData[sensorIndex].connected;
    sensor->lastDistance = sensorData[sensorIndex].lastDistance;

    return true;
}

/**
 * @brief รีเซ็ตข้อมูลทั้งหมด
 * เซ็ตค่าทั้งหมดเป็น 0
 */
void Data_ResetAll(void) {
    for (int i = 0; i < TOTAL_SENSORS; i++) {
        sensorData[i].count = 0;
        sensorData[i].connected = false;
        sensorData[i].lastDistance = 0;
    }
}

/**
 * @brief รีเซ็ตเซ็นเซอร์ตัวเดียว
 * @param sensorIndex หมายเลขเซ็นเซอร์ (0-3)
 * 
 * @return true = สำเร็จ, false = ล้มเหลว
 */
bool Data_ResetSensor(uint8_t sensorIndex) {
    if (sensorIndex >= TOTAL_SENSORS) {
        return false;
    }

    sensorData[sensorIndex].count = 0;
    sensorData[sensorIndex].connected = false;
    sensorData[sensorIndex].lastDistance = 0;

    return true;
}

/**
 * @brief ดึงข้อมูลสถานะระบบ
 * @param status ตัวแปรสำหรับเก็บข้อมูล
 * 
 * @return true = สำเร็จ, false = ล้มเหลว
 */
bool Data_GetSystemStatus(SystemStatus* status) {
    if (status == NULL) {
        return false;
    }

    status->wifiConnected = systemStatus.wifiConnected;
    status->lastUpdateTime = systemStatus.lastUpdateTime;

    return true;
}

/**
 * @brief ดึงจำนวนรวมของทั้งหมดเซ็นเซอร์
 * @return จำนวนรวม
 */
uint32_t Data_GetTotalCount(void) {
    uint32_t total = 0;
    for (int i = 0; i < TOTAL_SENSORS; i++) {
        total += sensorData[i].count;
    }
    return total;
}
