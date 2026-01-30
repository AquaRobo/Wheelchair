#ifndef SPI_HANDLER_H
#define SPI_HANDLER_H

#include <Arduino.h>
#include <ESP32SPISlave.h> // The default for the librarry is HSPI connection MOSI->13, MISO->12, SCLK->14, CS->15
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string.h>
#include "Commhandler.h"
// #include "ESP32SPISlaveMock.h"

class SPIHandler: public ICommHandler {
    public:
        SPIHandler(size_t BUFFER_SIZE, size_t QUEUE_SIZE);                                    
        void initialize() override;                 
        void send(const std::vector<uint8_t>& data) override;  // Insert the data type required to be sent
        std::vector<uint8_t> receive() override;    // Update the return type of the data received
        void close() override;
    private:
        ESP32SPISlave slave;
        size_t BUFFER_SIZE;
        size_t QUEUE_SIZE;
        uint8_t *tx_buf;
        uint8_t *rx_buf;
        size_t received_bytes = 0;
        std::vector<uint8_t> received_data;
        void initializeBuffers(uint8_t *tx, uint8_t *rx, size_t size, size_t offset = 0);      
};

#endif 