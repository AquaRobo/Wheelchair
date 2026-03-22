#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <stdlib.h>
// #include <math.h>

SPIHandler spi_handler(40, 4); // adjust the buffer size according to the size of the data being sent
float roll, pitch, yaw;
float orientation_x, orientation_y, orientation_z, orientation_w;
float linear_acc_x, linear_acc_y, linear_acc_z;
float angular_vel_x, angular_vel_y, angular_vel_z;
char cmd;
float v1, v2, v3;

void sensorTask(void* arg) {
    while (true) {
        // static float t = 0;
        // roll  = 30.0f * sinf(t);
        // pitch = 20.0f * sinf(t * 0.5f);
        // yaw   = 180.0f * sinf(t * 0.2f);
        // t += 0.05f;
        orientation_x = 0.707f; // example quaternion values for a 90-degree rotation around the Z-axis
        orientation_y = 0.0f;
        orientation_z = 0.707f;
        orientation_w = 0.0f;
        linear_acc_x = 0.0f; // example linear acceleration values
        linear_acc_y = 0.0f;
        linear_acc_z = 9.81f; // gravity
        angular_vel_x = 1.0f; // example angular velocity values
        angular_vel_y = 0.0f;
        angular_vel_z = 0.0f;

        // float imu_values[3] = {roll, pitch, yaw};
        float imu_values[10] = {orientation_x, orientation_y, orientation_z, orientation_w, linear_acc_x, linear_acc_y, linear_acc_z, angular_vel_x, angular_vel_y, angular_vel_z};
        uint8_t imu_payload[sizeof(imu_values)];
        memcpy(imu_payload, imu_values, sizeof(imu_values));
        spi_handler.fillTXBuffer(imu_payload, sizeof(imu_payload));
        vTaskDelay(20); // 100 Hz
        // Serial.printf("Sent IMU Data - Roll: %.2f, Pitch: %.2f, Yaw: %.2f\n", roll, pitch, yaw);
    }
}

void spiTask(void* arg) {
    while (true) {
        spi_handler.transferBuffers();
        auto actuators = spi_handler.getRXData();
        if (actuators.size() >= 9) {
            cmd = actuators[0];
            if (cmd == 'w'){
                memcpy(&v1, &actuators[4], 4);
                memcpy(&v2, &actuators[8], 4);
                Serial.printf("Received: %c, %.2f, %.2f\n", cmd, v1, v2);
            }
            else if (cmd == 's'){
                memcpy(&v1, &actuators[4], 4);
                memcpy(&v2, &actuators[8], 4);
                memcpy(&v3, &actuators[12], 4);
                Serial.printf("Received: %c, %.2f, %.2f, %.2f\n", cmd, v1, v2, v3);

            }
        }

        vTaskDelay(1);
    }
}

void setup() {
    Serial.begin(115200);
    spi_handler.initialize();

    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(spiTask, "SPITask", 4096, NULL, 2, NULL, 0);
    Serial.println("SPIHandler Initialized.");
}

void loop() {
}