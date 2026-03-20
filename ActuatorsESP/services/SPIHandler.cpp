#include "../headers/SPIHandler.h"
#include <iostream>

SPIHandler::SPIHandler(size_t BUFFER_SIZE, size_t QUEUE_SIZE){
    this->BUFFER_SIZE = BUFFER_SIZE;
    this->QUEUE_SIZE = QUEUE_SIZE;
    this->tx_buf_active = new uint8_t[BUFFER_SIZE];
    this->tx_buf_ready  = new uint8_t[BUFFER_SIZE];
    this->rx_buf        = new uint8_t[BUFFER_SIZE];

    memset(tx_buf_active, 0, BUFFER_SIZE);
    memset(tx_buf_ready, 0, BUFFER_SIZE);
    memset(rx_buf, 0, BUFFER_SIZE);
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
    this->initializeBuffers(this->tx_buf_ready, this->rx_buf, this->BUFFER_SIZE);
    size_t dataSize = std::min(data.size(), this->BUFFER_SIZE);
    for (size_t i = 0; i < dataSize; i++) {
        this->tx_buf_ready[i] = data[i];
    }
    this->slave.transfer(this->tx_buf_ready, this->rx_buf, this->BUFFER_SIZE);
}

std::vector<uint8_t> SPIHandler::receive(){
    // Fill tx buffer with dummy data
    this->initializeBuffers(this->tx_buf_active, this->rx_buf, this->BUFFER_SIZE);
    this->received_data.clear();
    this->received_bytes = this->slave.transfer(this->tx_buf_active, this->rx_buf, this->BUFFER_SIZE);
    // Convert received bytes to readable format aka vector
    this->received_data.reserve(this->received_bytes);
    for (size_t i = 0; i < this->received_bytes; i++) {
        this->received_data.push_back(this->rx_buf[i]);
    }
    return this->received_data;
}

void SPIHandler::close(){
    this->slave.end();
    delete[] this->tx_buf_active;
    delete[] this->tx_buf_ready;
    delete[] this->rx_buf;
}

void SPIHandler::fillTXBuffer(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        return;
    }

    size_t copy_size = std::min(size, BUFFER_SIZE);
    std::lock_guard<std::mutex> lock(buffer_mutex);
    memset(tx_buf_ready, 0, BUFFER_SIZE);
    memcpy(tx_buf_ready, data, copy_size);
}

size_t SPIHandler::transferBuffers() {
    // Swap buffers atomically
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        std::swap(tx_buf_active, tx_buf_ready);
    }

    // Queue SPI transfer (blocking until Pi clocks all bytes)
    received_bytes = slave.transfer(tx_buf_active, rx_buf, BUFFER_SIZE);

    // Copy received data into vector
    received_data.clear();
    received_data.reserve(received_bytes);
    for (size_t i = 0; i < received_bytes; i++)
        received_data.push_back(rx_buf[i]);

    return received_bytes;
}

std::vector<uint8_t> SPIHandler::getRXData() {
    return received_data;
}
