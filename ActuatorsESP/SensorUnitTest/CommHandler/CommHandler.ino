#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <stdlib.h>

SPIHandler spi_handler(20, 1); // adjust the buffer size according to the size of the data being sent
std::vector<uint8_t> all_data;

void setup(){
    Serial.begin(115200);
    spi_handler.initialize();
    Serial.println("spi Handler initialized.");
}

void loop(){
    all_data.clear();
    all_data = spi_handler.receive();
    char cmd = all_data[0];

    float v1, v2;
    memcpy(&v1, &all_data[1], 4);
    memcpy(&v2, &all_data[5], 4);

    Serial.print("CMD: ");
    Serial.println(cmd);
    Serial.print("V1: ");
    Serial.println(v1);
    Serial.print("V2: ");
    Serial.println(v2);
}