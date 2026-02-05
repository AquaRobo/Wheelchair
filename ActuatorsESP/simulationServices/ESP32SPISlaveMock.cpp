#include "../headers/ESP32SPISlaveMock.h"
#include <iostream>

using namespace std;

ESP32SPISlaveMock::ESP32SPISlaveMock() {
    cout << "ESP32SPISlaveMock initialized." << endl;
}

void ESP32SPISlaveMock::setDataMode(int mode) {
    cout << "Data mode set to: " << mode << endl;
}
void ESP32SPISlaveMock::setQueueSize(size_t size) {
    cout << "Queue size set to: " << size << endl;
}
void ESP32SPISlaveMock::begin() {
    cout << "ESP32SPISlaveMock started." << endl;
}
size_t ESP32SPISlaveMock::transfer(uint8_t *tx_buf, uint8_t *rx_buf, size_t size) {
    cout << "Transferring " << size << " bytes." << endl;
    // Mock transfer logic
    for (size_t i = 0; i < size; i++) {
        rx_buf[i] = tx_buf[i]; // Echo back the data
    }
    // Print the transferred data for debugging
    cout << "Transferred data: ";
    for (size_t i = 0; i < size; i++) {
        cout << static_cast<int>(rx_buf[i]) << " ";
    }
    cout << endl;
    return size; // Return number of bytes transferred
}
void ESP32SPISlaveMock::end() {
    cout << "ESP32SPISlaveMock ended." << endl;
}