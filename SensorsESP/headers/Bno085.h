#ifndef BNO085_H
#define BNO085_H

#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include "IIMU.h"

class Bno085 : public IIMU {
public:
    Bno085();
    std::vector<float> getGyroData() override;
    std::vector<float> getMagnetometerData() override;
    std::vector<float> getAccelerometerData() override;
    std::vector<float> getQuaternion() override;
    std::vector<float> getEulerAngles() override;
    void calibrate() override;
private:
    Adafruit_BNO08x bno08x;
    sh2_SensorValue_t sensor_value;
};

#endif