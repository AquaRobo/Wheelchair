#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <stdlib.h>

SPIHandler spi_handler(12, 1); // adjust the buffer size according to the size of the data being sent
std::vector<uint8_t> imu_data;
std::vector<uint8_t> all_data;
float roll, yaw, pitch;

void setup(){
    Serial.begin(115200);
    spi_handler.initialize();
    Serial.println("spi Handler initialized.");
}

void serializeFloatArray(std::vector<uint8_t> &buffer, float *array, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        uint8_t *p = reinterpret_cast<uint8_t *>(&array[i]);
        for (size_t j = 0; j < sizeof(float); ++j) {
            buffer.push_back(p[j]);
        }
    }
}

void loop(){
    imu_data.clear();
    all_data.clear();
    roll = 5.5;
    pitch = 6.5;
    yaw = 7.5;

    float imu_values[3] = {roll, pitch, yaw};
    serializeFloatArray(imu_data, imu_values, 4);

    all_data.insert(all_data.end(), imu_data.begin(), imu_data.end());
    
    Serial.print("IMU: ");
    for (int i = 0; i < 3; ++i) {
        Serial.print(imu_values[i]); Serial.print(" ");
    }
    Serial.println();
    
    spi_handler.send(all_data); 
}