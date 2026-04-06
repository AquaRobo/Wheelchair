#include "../headers/Bno085.h"

Bno085::Bno085(){
}

void Bno085::initialize(uint8_t i2c_address) {
    if (!this->bno08x.begin_I2C(i2c_address)) {
        this->initialized = false;
        Serial.println("Failed to initialize BNO085 sensor");
        return;
    }

    bool reports_ok = true;
    reports_ok &= this->bno08x.enableReport(SH2_ACCELEROMETER);
    reports_ok &= this->bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED);
    reports_ok &= this->bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED);
    reports_ok &= this->bno08x.enableReport(SH2_GAME_ROTATION_VECTOR);

    this->initialized = reports_ok;
    this->has_gyro = false;
    this->has_magnetometer = false;
    this->has_accelerometer = false;
    this->has_quaternion = false;

    this->latest_gyro.clear();
    this->latest_magnetometer.clear();
    this->latest_accelerometer.clear();
    this->latest_quaternion.clear();

    if (reports_ok) {
        Serial.println("BNO085 sensor initialized successfully");
    } else {
        Serial.println("BNO085 initialized but one or more reports failed");
    }
}

bool Bno085::isInitialized() const {
    return this->initialized;
}

bool Bno085::updateCachedData(uint8_t max_events) {
    if (!this->initialized) {
        return false;
    }

    bool got_any_event = false;
    for (uint8_t i = 0; i < max_events; ++i) {
        if (!this->bno08x.getSensorEvent(&this->sensor_value)) {
            break;
        }

        got_any_event = true;
        switch (this->sensor_value.sensorId) {
            case SH2_GYROSCOPE_CALIBRATED:
                this->latest_gyro = {
                    this->sensor_value.un.gyroscope.x,
                    this->sensor_value.un.gyroscope.y,
                    this->sensor_value.un.gyroscope.z
                };
                this->has_gyro = true;
                break;

            case SH2_MAGNETIC_FIELD_CALIBRATED:
                this->latest_magnetometer = {
                    this->sensor_value.un.magneticField.x,
                    this->sensor_value.un.magneticField.y,
                    this->sensor_value.un.magneticField.z
                };
                this->has_magnetometer = true;
                break;

            case SH2_ACCELEROMETER:
                this->latest_accelerometer = {
                    this->sensor_value.un.accelerometer.x,
                    this->sensor_value.un.accelerometer.y,
                    this->sensor_value.un.accelerometer.z
                };
                this->has_accelerometer = true;
                break;

            case SH2_GAME_ROTATION_VECTOR:
                this->latest_quaternion = {
                    this->sensor_value.un.gameRotationVector.real,
                    this->sensor_value.un.gameRotationVector.i,
                    this->sensor_value.un.gameRotationVector.j,
                    this->sensor_value.un.gameRotationVector.k
                };
                this->has_quaternion = true;
                break;

            default:
                break;
        }
    }

    return got_any_event;
}

std::vector<float> Bno085::getGyroData() {
    if (!this->has_gyro) {
        this->updateCachedData();
    }
    return this->has_gyro ? this->latest_gyro : std::vector<float>{};
}

std::vector<float> Bno085::getMagnetometerData() {
    if (!this->has_magnetometer) {
        this->updateCachedData();
    }
    return this->has_magnetometer ? this->latest_magnetometer : std::vector<float>{};
}

std::vector<float> Bno085::getAccelerometerData() {
    if (!this->has_accelerometer) {
        this->updateCachedData();
    }
    return this->has_accelerometer ? this->latest_accelerometer : std::vector<float>{};
}

std::vector<float> Bno085::getQuaternion() {
    if (!this->has_quaternion) {
        this->updateCachedData();
    }
    return this->has_quaternion ? this->latest_quaternion : std::vector<float>{};
}

std::vector<float> Bno085::getEulerAngles(bool degrees) {
    if (!this->has_quaternion) {
        this->updateCachedData();
    }
    if (this->has_quaternion) {
        float real = this->latest_quaternion[0];
        float i = this->latest_quaternion[1];
        float j = this->latest_quaternion[2];
        float k = this->latest_quaternion[3];

        float sq_real = real * real;
        float sq_i = i * i;
        float sq_j = j * j;
        float sq_k = k * k;

        float yaw = atan2f(2.0f * (i * j + real * k), sq_i - sq_j - sq_k + sq_real);
        float pitch = asinf(-2.0f * (i * k - real * j));
        float roll = atan2f(2.0f * (j * k + real * i), -sq_i - sq_j + sq_k + sq_real);
        if (degrees) {
            yaw *= 180.0f / PI;
            pitch *= 180.0f / PI;
            roll *= 180.0f / PI;
        }
        return {roll, pitch, yaw};
    }
    return {};
}

void Bno085::calibrate(){
    
}