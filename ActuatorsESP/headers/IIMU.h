#ifndef IIMU_H
#define IIMU_H

#include <vector>
#include <cstdint>

class IIMU {
public:
    virtual std::vector<float> getGyroData() = 0;
    virtual std::vector<float> getMagnetometerData() = 0;
    virtual std::vector<float> getAccelerometerData() = 0;
    virtual std::vector<float> getQuaternion() = 0;
    virtual std::vector<float> getEulerAngles() = 0;
    virtual void calibrate() = 0;
    virtual ~IIMU() {}
};

#endif