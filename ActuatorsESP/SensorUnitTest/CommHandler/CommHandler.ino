#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <stdlib.h>
#include <math.h>

SPIHandler spi_handler(32, 4); // adjust the buffer size according to the size of the data being sent
float roll, yaw, pitch;
char cmd;
float v1, v2, v3;

void sensorTask(void* arg) {
    while (true) {
        static float t = 0;

        roll  = 30.0f * sinf(t);
        pitch = 20.0f * sinf(t * 0.5f);
        yaw   = 180.0f * sinf(t * 0.2f);

        t += 0.05f;

        float imu_values[3] = {roll, pitch, yaw};
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