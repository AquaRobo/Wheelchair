#include "../headers/SPIHandler.h"

SPIHandler::SPIHandler(size_t BUFFER_SIZE = 8, size_t QUEUE_SIZE = 1){
    this->BUFFER_SIZE = BUFFER_SIZE;
    this->QUEUE_SIZE = QUEUE_SIZE;
    this->tx_buf = new uint8_t[BUFFER_SIZE];
    this->rx_buf = new uint8_t[BUFFER_SIZE];
}

void SPIHandler::initialize(){
    this->slave.setDataMode(0); // Understand SPI modes --> SPI_MODE0
    this->slave.setQueueSize(this->QUEUE_SIZE);
    this->slave.begin(HSPI);
}

void SPIHandler::initializeBuffers(uint8_t *tx, uint8_t *rx, size_t size, size_t offset){
    if (tx) memset(tx, 0, size);
    if (rx) memset(rx, 0, size);
}

void SPIHandler::send(const std::vector<uint8_t>& data){
    // Fill tx buffer with data
    this->initializeBuffers(this->tx_buf, this->rx_buf, this->BUFFER_SIZE);
    size_t dataSize = std::min(data.size(), this->BUFFER_SIZE);
    for (size_t i = 0; i < dataSize; i++) {
        this->tx_buf[i] = data[i];
    }
    this->slave.transfer(this->tx_buf, this->rx_buf, this->BUFFER_SIZE);
    memset(this->tx_buf, 0, this->BUFFER_SIZE);
    
}

std::vector<uint8_t> SPIHandler::receive(){
    // Fill tx buffer with dummy data
    this->initializeBuffers(this->tx_buf, this->rx_buf, this->BUFFER_SIZE);
    this->received_bytes = this->slave.transfer(this->tx_buf, this->rx_buf, this->BUFFER_SIZE);
    // Convert received bytes to readable format aka vector
    this->received_data.reserve(this->received_bytes);
    for (size_t i = 0; i < this->received_bytes; i++) {
        this->received_data.push_back(this->rx_buf[i]);
    }
    return this->received_data;
}

void SPIHandler::close(){
    this->slave.end();
    delete[] this->tx_buf;
    delete[] this->rx_buf;
}
