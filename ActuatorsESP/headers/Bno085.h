#ifndef BNO085_H
#define BNO085_H

#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include "IIMU.h"

class Bno085 : public IIMU {
public:
    Bno085();
    void initialize(uint8_t i2c_address = 0x4B);
    bool isInitialized() const;
    bool updateCachedData(uint8_t max_events = 10);
    std::vector<float> getGyroData() override;
    std::vector<float> getMagnetometerData() override;
    std::vector<float> getAccelerometerData() override;
    std::vector<float> getQuaternion() override;
    std::vector<float> getEulerAngles(bool degrees = false) override;
    void calibrate() override;
private:
    Adafruit_BNO08x bno08x;
    sh2_SensorValue_t sensor_value;
    bool initialized = false;
    std::vector<float> latest_gyro;
    std::vector<float> latest_magnetometer;
    std::vector<float> latest_accelerometer;
    std::vector<float> latest_quaternion;
    bool has_gyro = false;
    bool has_magnetometer = false;
    bool has_accelerometer = false;
    bool has_quaternion = false;
};

#endif