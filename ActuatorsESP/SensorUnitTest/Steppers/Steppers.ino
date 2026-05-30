#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <AccelStepper.h>
#include <stdlib.h>

// -------------------------------------------------------
// SPI
// -------------------------------------------------------
SPIHandler spi_handler(40, 4);

// -------------------------------------------------------
// Pin Definitions
// -------------------------------------------------------
#define STEP_SHOULDER_R   14    // stepper1 - Base Right
#define DIR_SHOULDER_R    27

#define STEP_SHOULDER_L   12    // stepper2 - Base Left  (not connected yet)
#define DIR_SHOULDER_L    15

#define STEP_GEARED 33    // stepper3 - Shoulder (with 5.18:1 gearbox)
#define DIR_GEARED  4

#define STEP_FOREARM  26    // stepper4 - Forearm
#define DIR_FOREARM   25

#define STEP_WRIST    32    // stepper5 - Wrist
#define DIR_WRIST     16

#define STEP_ROTATION 36  // stepper6 - Base Rotation
#define DIR_ROTATION  2

// ------------------------------------------------------------
// Motor Speed & Acceleration Parameters
// ------------------------------------------------------------
// Microstepping = 16  .  3200 steps/rev (motor shaft)
// Gearbox (stepper3)  .  3200 * 5.18 = 16576 steps/rev output
//
// Target motor speed: 150 RPM (sweet spot for NEMA17 torque & silence)
//   Standard joints:  150 RPM * 3200 / 60 =  8000 steps/s
//   Shoulder (geared): same motor speed, gearbox handles load
//
// Acceleration: ramp to max in ~0.5s
//   8000 steps/s / 0.5s = 16000 steps/s^2
// ------------------------------------------------------------
#define MICROSTEPS          16
#define STEPS_PER_REV       (200 * MICROSTEPS)   // 3200

#define SPEED_STANDARD      8000    // steps/s  (~150 RPM at motor shaft)
#define SPEED_WRIST         300    // steps/s  (wrist needs less speed)
#define SPEED_ROTATION      6000    // steps/s  (base rotation)

#define ACCEL_STANDARD      16000   // steps/s^2 (0 to max in ~0.5s)
#define ACCEL_WRIST         600
#define ACCEL_ROTATION      12000
// -------------------------------------------------------
// Stepper Motors
// -------------------------------------------------------
AccelStepper motorShoulderR   (AccelStepper::DRIVER, STEP_SHOULDER_R,   DIR_SHOULDER_R);
AccelStepper motorShoulderL   (AccelStepper::DRIVER, STEP_SHOULDER_L,   DIR_SHOULDER_L);
AccelStepper motorGeared(AccelStepper::DRIVER, STEP_GEARED, DIR_GEARED);
AccelStepper motorForearm (AccelStepper::DRIVER, STEP_FOREARM,  DIR_FOREARM);
AccelStepper motorWrist   (AccelStepper::DRIVER, STEP_WRIST,    DIR_WRIST);
AccelStepper motorRotation(AccelStepper::DRIVER, STEP_ROTATION, DIR_ROTATION);

AccelStepper* steppers[] = {
  &motorRotation,
  &motorShoulderR,
  &motorShoulderL,
  &motorGeared,
  &motorForearm,
  &motorWrist,
};



// -------------------------------------------------------
// Globals
// -------------------------------------------------------
char cmd;
float v1, v2;

// -------------------------------------------------------
//sensors
//--------------------------------------------------------
float roll, pitch, yaw;
float orientation_x, orientation_y, orientation_z, orientation_w;
float linear_acc_x, linear_acc_y, linear_acc_z;
float angular_vel_x, angular_vel_y, angular_vel_z;

void sensorTask(void* arg) {
    while (true) {
        // static float t = 0;
        // roll  = 30.0f * sinf(t);
        // pitch = 20.0f * sinf(t * 0.5f);
        // yaw   = 180.0f * sinf(t * 0.2f);
        // t += 0.05f;
        orientation_x = 0.707f; // example quaternion values for a 90-degree rotation around the Z-axis
        orientation_y = 0.0f;
        orientation_z = 0.707f;
        orientation_w = 0.0f;
        linear_acc_x = 0.0f; // example linear acceleration values
        linear_acc_y = 0.0f;
        linear_acc_z = 9.81f; // gravity
        angular_vel_x = 1.0f; // example angular velocity values
        angular_vel_y = 0.0f;
        angular_vel_z = 0.0f;

        // float imu_values[3] = {roll, pitch, yaw};
        float imu_values[10] = {orientation_x, orientation_y, orientation_z, orientation_w, linear_acc_x, linear_acc_y, linear_acc_z, angular_vel_x, angular_vel_y, angular_vel_z};
        uint8_t imu_payload[sizeof(imu_values)];
        memcpy(imu_payload, imu_values, sizeof(imu_values));
        spi_handler.fillTXBuffer(imu_payload, sizeof(imu_payload));
        vTaskDelay(20); // 100 Hz
        // Serial.printf("Sent IMU Data - Roll: %.2f, Pitch: %.2f, Yaw: %.2f\n", roll, pitch, yaw);
    }
}

// -------------------------------------------------------
// actuatorTask — handles SPI and updates motor targets.
// transferBuffers() blocks until the Pi clocks a full
// transaction, so this task cannot also run steppers.
// It only calls moveTo() when a target actually changes;
// stepperTask drives the motors from there.
// -------------------------------------------------------
void actuatorTask(void* arg) {
    static int32_t prevSteps[6] = {0, 0, 0, 0, 0, 0};

    while (true) {
        spi_handler.transferBuffers();
        auto actuators = spi_handler.getRXData();

        if (actuators.size() >= 4) {
            cmd = actuators[0];

            // 'w' command — receives 2 floats (e.g. wheel speeds)
            if (cmd == 'w' && actuators.size() >= 9) {
                memcpy(&v1, &actuators[4], 4);
                memcpy(&v2, &actuators[8], 4);
                Serial.printf("Received: %c, %.2f, %.2f\n", cmd, v1, v2);
            }

            // 's' command — absolute step targets for each joint.
            // The ROS node keeps republishing the same value, so guard
            // with prevSteps to avoid redundant moveTo() calls.
            else if (cmd == 's' && actuators.size() >= 28) {
                int32_t newSteps[6];
                memcpy(&newSteps[0], &actuators[4],  4);
                memcpy(&newSteps[1], &actuators[8],  4);
                memcpy(&newSteps[2], &actuators[12], 4);
                memcpy(&newSteps[3], &actuators[16], 4);
                memcpy(&newSteps[4], &actuators[20], 4);
                memcpy(&newSteps[5], &actuators[24], 4);

                if (newSteps[0] != prevSteps[0]) {
                    motorRotation.moveTo(newSteps[0]);
                    prevSteps[0] = newSteps[0];
                }
                if (newSteps[1] != prevSteps[1]) {
                    motorShoulderR.moveTo( newSteps[1]);
                    motorShoulderL.moveTo(-newSteps[1]);
                    prevSteps[1] = newSteps[1];
                }
                if (newSteps[2] != prevSteps[2]) {
                    motorGeared.moveTo(newSteps[2]);
                    prevSteps[2] = newSteps[2];
                }
                if (newSteps[3] != prevSteps[3]) {
                    motorForearm.moveTo(newSteps[3]);
                    prevSteps[3] = newSteps[3];
                }
                if (newSteps[4] != prevSteps[4]) {
                    motorWrist.moveTo(newSteps[4]);
                    prevSteps[4] = newSteps[4];
                }

                Serial.printf("Targets → M1:%d M2:%d M3:%d M4:%d M5:%d\n",
                              newSteps[0], newSteps[1], newSteps[2],
                              newSteps[3], newSteps[4]);
                for (int i = 0; i < 6; i++) {
                    motorRotation.run();
                    motorShoulderR.run();
                    motorShoulderL.run();
                    motorGeared.run();
                    motorForearm.run();
                    motorWrist.run();
                }           
            }
        }
    }
}


// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {
    Serial.begin(115200);

    // Standard arm joints
    motorShoulderR.setMaxSpeed(SPEED_STANDARD);
    motorShoulderR.setAcceleration(ACCEL_STANDARD);

    motorShoulderL.setMaxSpeed(SPEED_STANDARD);
    motorShoulderL.setAcceleration(ACCEL_STANDARD);

    motorGeared.setMaxSpeed(SPEED_STANDARD);
    motorGeared.setAcceleration(ACCEL_STANDARD);

    motorForearm.setMaxSpeed(SPEED_STANDARD);
    motorForearm.setAcceleration(ACCEL_STANDARD);

    // Wrist — finer, slower
    motorWrist.setMaxSpeed(SPEED_WRIST);
    motorWrist.setAcceleration(ACCEL_WRIST);

    // Base rotation
    motorRotation.setMaxSpeed(SPEED_ROTATION);
    motorRotation.setAcceleration(ACCEL_ROTATION);

    spi_handler.initialize();

    // core 0: actuatorTask (SPI + target updates)
    // core 1: stepperTask (continuous run() polling) + sensorTask (sleeps 20ms)
    xTaskCreatePinnedToCore(actuatorTask, "actuatorTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask,   "sensorTask",   4096, NULL, 1, NULL, 1);

    Serial.println("SPIHandler Initialized.");
}

// -------------------------------------------------------
// Loop — empty, everything runs in FreeRTOS tasks
// -------------------------------------------------------
void loop() {}