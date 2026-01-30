#include "headers/SPIHandler.h"
#include "services/SPIHandler.cpp"

SPIHandler spi_handler(40, 1); // adjust the buffer size according to the size of the data being sent
std::vector<uint8_t> data;

void setup(){
    spi_handler.initialize();

    while (true){
        pins = spi_handler.receive();
        if (!pins.empty()){
            // look for the keyword at the end of the pins vector '40'
            if (pins.back() == 40){
                pins.pop_back(); // Remove the keyword '40'
                for (const auto &pin : pins) pinMode(pin, OUTPUT);
                spi_handler.send({0x4F, 0x4B}); // 'OK' in hex
                break;                          
            }
            else delay(50);
        }
    }
}

void loop()
{
    // append your data in the vector
    spi_handler.send(data);
}
