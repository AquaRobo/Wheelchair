#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <stdlib.h>

SPIHandler spi_handler(28, 1); // adjust the buffer size according to the size of the data being sent
std::vector<uint8_t> all_data;

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
    all_data.clear();

}