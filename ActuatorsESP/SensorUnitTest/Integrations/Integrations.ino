#include "../../headers/SPIHandler.h"
#include "../../headers/Bno085.h"
#include "../../services/SPIHandler.cpp"
#include "../../services/Bno085.cpp"
#include <stdlib.h>
#include <Wire.h>
// #include <math.h>

SPIHandler spi_handler(40, 4); // adjust the buffer size according to the size of the data being sent
Bno085 bno085;
std::vector<float> gyro_data;
std::vector<float> accelerometer_data;
std::vector<float> quaternion;

float roll, pitch, yaw;
float orientation_i, orientation_j, orientation_k, orientation_w;
float linear_acc_x, linear_acc_y, linear_acc_z;
float angular_vel_x, angular_vel_y, angular_vel_z;
char cmd;
float v1, v2, v3;

void sensorTask(void* arg) {
    while (true) {
        bno085.updateCachedData(12);
        gyro_data = bno085.getGyroData();
        accelerometer_data = bno085.getAccelerometerData();
        quaternion = bno085.getQuaternion();
        if (gyro_data.empty() || accelerometer_data.empty() || quaternion.empty()) {
            Serial.println("Waiting for complete BNO085 sample...");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        orientation_i = quaternion[1];
        orientation_j = quaternion[2];
        orientation_k = quaternion[3];
        orientation_w = quaternion[0];
        linear_acc_x = accelerometer_data[0];
        linear_acc_y = accelerometer_data[1];
        linear_acc_z = accelerometer_data[2];
        angular_vel_x = gyro_data[0];
        angular_vel_y = gyro_data[1];
        angular_vel_z = gyro_data[2];

        float imu_values[10] = {orientation_i, orientation_j, orientation_k, orientation_w, linear_acc_x, linear_acc_y, linear_acc_z, angular_vel_x, angular_vel_y, angular_vel_z};
        uint8_t imu_payload[sizeof(imu_values)];
        memcpy(imu_payload, imu_values, sizeof(imu_values));
        spi_handler.fillTXBuffer(imu_payload, sizeof(imu_payload));
        vTaskDelay(pdMS_TO_TICKS(10)); // 100 Hz
    }
}

void spiTask(void* arg) {
    while (true) {
        spi_handler.transferBuffers();
        auto actuators = spi_handler.getRXData();
        if (!actuators.empty()) {
            cmd = actuators[0];
            if (cmd == 'w') {
                memcpy(&v1, &actuators[4], 4);
                memcpy(&v2, &actuators[8], 4);
                Serial.printf("Received: %c, %.2f, %.2f\n", cmd, v1, v2);
            }
            else if (cmd == 's') {
                memcpy(&v1, &actuators[4], 4);
                memcpy(&v2, &actuators[8], 4);
                memcpy(&v3, &actuators[12], 4);
                Serial.printf("Received: %c, %.2f, %.2f, %.2f\n", cmd, v1, v2, v3);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(100);
    spi_handler.initialize();
    bno085.initialize(0x4B);

    if (!bno085.isInitialized()) {
        Serial.println("BNO085 initialization failed - check wiring/address");
        while(1) delay(10);
    }

    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(spiTask, "SPITask", 4096, NULL, 2, NULL, 0);
    Serial.println("SPIHandler Initialized.");
}

void loop() {
}