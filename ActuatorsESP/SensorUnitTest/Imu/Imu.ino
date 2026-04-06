#include "../../headers/Bno085.h"
#include "../../services/Bno085.cpp"
#include <stdlib.h>

Bno085 bno085;
std::vector<float> gyro_data;
std::vector<float> magnetometer_data;
std::vector<float> accelerometer_data;
std::vector<float> quaternion;
std::vector<float> euler_angles;


void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("BNO085 IMU Test");

    bno085.initialize(0x4B);

    if (!bno085.isInitialized()) {
        Serial.println("BNO085 initialization failed - check wiring/address");
        while(1) delay(10);
    }

}

void loop() {
    bno085.updateCachedData(12);

    gyro_data = bno085.getGyroData();
    magnetometer_data = bno085.getMagnetometerData();
    accelerometer_data = bno085.getAccelerometerData();
    quaternion = bno085.getQuaternion();
    euler_angles = bno085.getEulerAngles(true);
    if (gyro_data.empty() || accelerometer_data.empty() || quaternion.empty() || euler_angles.empty() || magnetometer_data.empty()) {
        Serial.println("Waiting for complete BNO085 sample...");
        delay(10);
        return;
    }
    
    Serial.print("Gyro: ");
    for (const auto &value : gyro_data) {
        Serial.print(value); Serial.print(" ");
    }
    Serial.println();

    Serial.print("Magnetometer: ");
    for (const auto &value : magnetometer_data) {
        Serial.print(value); Serial.print(" ");
    }
    Serial.println();

    Serial.print("Accelerometer: ");
    for (const auto &value : accelerometer_data) {
        Serial.print(value); Serial.print(" ");
    }
    Serial.println();

    Serial.print("Quaternion: ");
    for (const auto &value : quaternion) {
        Serial.print(value); Serial.print(" ");
    }
    Serial.println();

    Serial.print("Euler Angles: ");
    for (const auto &value : euler_angles) {
        Serial.print(value); Serial.print(" ");
    }
    Serial.println();

    delay(50);
}