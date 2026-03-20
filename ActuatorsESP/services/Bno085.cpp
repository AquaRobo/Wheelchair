#include "../headers/Bno085.h"
#include <iostream>

Bno085::Bno085(){
    if (!this->bno08x.begin_I2C()) {
        std::cerr << "Failed to initialize BNO085 sensor!" << std::endl;
    }
    this->bno08x.enableReport(SH2_ACCELEROMETER);
    this->bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED);
    this->bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED);
    this->bno08x.enableReport(SH2_GAME_ROTATION_VECTOR);

    std::cout << "BNO085 sensor initialized successfully." << std::endl;
}

std::vector<float> Bno085::getGyroData() {
    if (this->bno08x.getSensorEvent(&this->sensor_value) && this->sensor_value.sensorId == SH2_GYROSCOPE_CALIBRATED) {
        return {this->sensor_value.un.gyroscope.x, this->sensor_value.un.gyroscope.y, this->sensor_value.un.gyroscope.z};
    }
    return {};
}

std::vector<float> Bno085::getMagnetometerData() {
    if (this->bno08x.getSensorEvent(&this->sensor_value) && this->sensor_value.sensorId == SH2_MAGNETIC_FIELD_CALIBRATED) {
        return {this->sensor_value.un.magneticField.x, this->sensor_value.un.magneticField.y, this->sensor_value.un.magneticField.z};
    }
    return {};
}

std::vector<float> Bno085::getAccelerometerData() {
    if (this->bno08x.getSensorEvent(&this->sensor_value) && this->sensor_value.sensorId == SH2_ACCELEROMETER) {
        return {this->sensor_value.un.accelerometer.x, this->sensor_value.un.accelerometer.y, this->sensor_value.un.accelerometer.z};
    }
    return {};
}

std::vector<float> Bno085::getQuaternion() {
    if (this->bno08x.getSensorEvent(&this->sensor_value) && this->sensor_value.sensorId == SH2_GAME_ROTATION_VECTOR) {
        return {this->sensor_value.un.gameRotationVector.real, this->sensor_value.un.gameRotationVector.i, this->sensor_value.un.gameRotationVector.j, this->sensor_value.un.gameRotationVector.k};
    }
    return {};
}

std::vector<float> Bno085::getEulerAngles() {
    if (this->bno08x.getSensorEvent(&this->sensor_value) && this->sensor_value.sensorId == SH2_GAME_ROTATION_VECTOR) {
        float sq_real = this->sensor_value.un.gameRotationVector.real * this->sensor_value.un.gameRotationVector.real;
        float sq_i = this->sensor_value.un.gameRotationVector.i * this->sensor_value.un.gameRotationVector.i;
        float sq_j = this->sensor_value.un.gameRotationVector.j * this->sensor_value.un.gameRotationVector.j;
        float sq_k = this->sensor_value.un.gameRotationVector.k * this->sensor_value.un.gameRotationVector.k;

        float yaw = atan2f(2.0f * (this->sensor_value.un.gameRotationVector.i * this->sensor_value.un.gameRotationVector.j + this->sensor_value.un.gameRotationVector.real * this->sensor_value.un.gameRotationVector.k), sq_i - sq_j - sq_k + sq_real);
        float pitch = asinf(-2.0f * (this->sensor_value.un.gameRotationVector.i * this->sensor_value.un.gameRotationVector.k - this->sensor_value.un.gameRotationVector.real * this->sensor_value.un.gameRotationVector.j));
        float roll = atan2f(2.0f * (this->sensor_value.un.gameRotationVector.j * this->sensor_value.un.gameRotationVector.k + this->sensor_value.un.gameRotationVector.real * this->sensor_value.un.gameRotationVector.i), -sq_i - sq_j + sq_k + sq_real);
        return {roll, pitch, yaw};
    }
    return {};
}

void Bno085::calibrate(){
    
}