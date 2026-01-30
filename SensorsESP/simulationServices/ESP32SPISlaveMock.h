#ifndef ESP32SPISLAVEMOCK_H
#define ESP32SPISLAVEMOCK_H

#include <cstddef>
#include <cstdint>
#include <cstring>

class ESP32SPISlaveMock {
    public:
        ESP32SPISlaveMock();
        void setDataMode(int mode);
        void setQueueSize(size_t size);
        void begin();
        size_t transfer(uint8_t *tx_buf, uint8_t *rx_buf, size_t size);
        void end();
};
#endif