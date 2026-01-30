#ifndef COMMHANDLER_H
#define COMMHANDLER_H

#include <vector>
#include <cstdint>

class ICommHandler
{
public:
    virtual void initialize() = 0;
    virtual void send(const std::vector<uint8_t> &data) = 0;
    virtual std::vector<uint8_t> receive() = 0;
    virtual void close() = 0;
    virtual ~ICommHandler() {}
};

#endif
