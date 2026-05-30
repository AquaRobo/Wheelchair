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
#define STEP_SHOULDER_R   14    // stepper1 - Shoulder Right
#define DIR_SHOULDER_R    27

#define STEP_SHOULDER_L   12    // stepper2 - Shoulder Left (mirrored)
#define DIR_SHOULDER_L    15

#define STEP_GEARED       33    // stepper3 - Geared shoulder (5.18:1 gearbox)
#define DIR_GEARED         4

#define STEP_FOREARM      26    // stepper4 - Forearm
#define DIR_FOREARM       25

#define STEP_WRIST        32    // stepper5 - Wrist
#define DIR_WRIST         16

#define STEP_ROTATION     36    // stepper6 - Base Rotation
#define DIR_ROTATION       2

// -------------------------------------------------------
// Motor Speed & Acceleration Parameters
// -------------------------------------------------------
// Driver microstep: 16  →  3200 steps/rev (motor shaft)
// Gearbox (stepper3): 3200 * 5.18 = 16,576 steps/rev at output
//
// Target motor speed: 150 RPM
//   Standard joints:  150 * 3200 / 60 =  8000 steps/s
//   Geared joint:     same motor speed; gearbox handles the load
//
// Acceleration: 0 → max in ~0.5 s
//   8000 / 0.5 = 16,000 steps/s²
// -------------------------------------------------------
#define MICROSTEPS          16
#define STEPS_PER_REV       (200 * MICROSTEPS)          // 3200 — motor shaft

#define SPEED_STANDARD      8000    // steps/s  (~150 RPM)
#define SPEED_WRIST          300    // steps/s  (fine wrist movement)
#define SPEED_ROTATION      6000    // steps/s  (base rotation)

#define ACCEL_STANDARD     16000    // steps/s²
#define ACCEL_WRIST          600
#define ACCEL_ROTATION     12000

// -------------------------------------------------------
// Stepper Motors
// -------------------------------------------------------
AccelStepper motorShoulderR(AccelStepper::DRIVER, STEP_SHOULDER_R, DIR_SHOULDER_R);
AccelStepper motorShoulderL(AccelStepper::DRIVER, STEP_SHOULDER_L, DIR_SHOULDER_L);
AccelStepper motorGeared   (AccelStepper::DRIVER, STEP_GEARED,     DIR_GEARED);
AccelStepper motorForearm  (AccelStepper::DRIVER, STEP_FOREARM,    DIR_FOREARM);
AccelStepper motorWrist    (AccelStepper::DRIVER, STEP_WRIST,      DIR_WRIST);
AccelStepper motorRotation (AccelStepper::DRIVER, STEP_ROTATION,   DIR_ROTATION);

// -------------------------------------------------------
// Mutex — protects moveTo() / run() from concurrent access
// between actuatorTask (Core 0) and stepperTask (Core 1)
// -------------------------------------------------------
SemaphoreHandle_t motorMutex;

// -------------------------------------------------------
// Globals
// -------------------------------------------------------
char  cmd;
float v1, v2;

// -------------------------------------------------------
// Sensor data
// -------------------------------------------------------
float orientation_x, orientation_y, orientation_z, orientation_w;
float linear_acc_x,  linear_acc_y,  linear_acc_z;
float angular_vel_x, angular_vel_y, angular_vel_z;

// =======================================================
// sensorTask — fills TX buffer with IMU data at 50 Hz
// Runs on Core 1 (low priority, sleeps between updates)
// =======================================================
void sensorTask(void* arg) {
    while (true) {
        // Replace with real IMU reads when sensor is wired up
        orientation_x = 0.707f;
        orientation_y = 0.0f;
        orientation_z = 0.707f;
        orientation_w = 0.0f;
        linear_acc_x  = 0.0f;
        linear_acc_y  = 0.0f;
        linear_acc_z  = 9.81f;
        angular_vel_x = 1.0f;
        angular_vel_y = 0.0f;
        angular_vel_z = 0.0f;

        float imu_values[10] = {
            orientation_x, orientation_y, orientation_z, orientation_w,
            linear_acc_x,  linear_acc_y,  linear_acc_z,
            angular_vel_x, angular_vel_y, angular_vel_z
        };
        uint8_t imu_payload[sizeof(imu_values)];
        memcpy(imu_payload, imu_values, sizeof(imu_values));
        spi_handler.fillTXBuffer(imu_payload, sizeof(imu_payload));

        vTaskDelay(pdMS_TO_TICKS(20));   // 50 Hz
    }
}

// =======================================================
// stepperTask — calls run() on every motor in a tight loop
//
// This is the CRITICAL fix. AccelStepper::run() must be
// called thousands of times per second to generate the
// step pulses. In the broken version it was only called
// 6 times per SPI packet (every 100 ms) — far too rarely
// for the motors to move meaningfully.
//
// Runs on Core 1 with the highest priority so it is never
// starved by sensorTask or idle work.
// =======================================================
void stepperTask(void* arg) {
    while (true) {
        // Take mutex briefly so moveTo() calls in actuatorTask
        // cannot race with run() here.
        if (xSemaphoreTake(motorMutex, 0) == pdTRUE) {
            motorRotation.run();
            motorShoulderR.run();
            motorShoulderL.run();
            motorGeared.run();
            motorForearm.run();
            motorWrist.run();
            xSemaphoreGive(motorMutex);
        }
        // No delay — must be a tight loop to hit step-pulse timing
    }
}

// =======================================================
// actuatorTask — receives SPI packets and updates motor
// targets via moveTo().
//
// Only calls moveTo() when a target actually changes so
// the stepper's acceleration profile is not reset on every
// identical 100 ms packet from the ROS node.
//
// Runs on Core 0.
// =======================================================
void actuatorTask(void* arg) {
    // Cache of the last commanded absolute step targets.
    // Initialise to 0 so the first packet always triggers moveTo().
    static int32_t prevSteps[6] = {0, 0, 0, 0, 0, 0};

    while (true) {
        // Blocks until the Pi clocks a full SPI transaction
        spi_handler.transferBuffers();
        auto actuators = spi_handler.getRXData();

        if (actuators.size() < 4) continue;

        cmd = actuators[0];

        // ---------------------------------------------------
        // 'w' command — two wheel/servo floats
        // ---------------------------------------------------
        if (cmd == 'w' && actuators.size() >= 9) {
            memcpy(&v1, &actuators[4], 4);
            memcpy(&v2, &actuators[8], 4);
            Serial.printf("Received: %c, %.2f, %.2f\n", cmd, v1, v2);
        }

        // ---------------------------------------------------
        // 's' command — 6 absolute step targets (int32_t each)
        //
        // Packet layout (bytes):
        //   [0]      char  cmd  = 's'
        //   [1..3]   padding / unused
        //   [4..7]   int32  joint0 (Rotation)
        //   [8..11]  int32  joint1 (Shoulder differential)
        //   [12..15] int32  joint2 (Geared shoulder)
        //   [16..19] int32  joint3 (Forearm)
        //   [20..23] int32  joint4 (Wrist)
        //   [24..27] int32  joint5 (unused / spare)
        // ---------------------------------------------------
        else if (cmd == 's' && actuators.size() >= 28) {
            int32_t newSteps[6];
            memcpy(&newSteps[0], &actuators[4],  4);
            memcpy(&newSteps[1], &actuators[8],  4);
            memcpy(&newSteps[2], &actuators[12], 4);
            memcpy(&newSteps[3], &actuators[16], 4);
            memcpy(&newSteps[4], &actuators[20], 4);
            memcpy(&newSteps[5], &actuators[24], 4);

            // Only call moveTo() when the target actually changed.
            // This prevents resetting AccelStepper's velocity profile
            // on every repeated packet from the 100 ms ROS timer.
            xSemaphoreTake(motorMutex, portMAX_DELAY);

            if (newSteps[0] != prevSteps[0]) {
                motorRotation.moveTo(newSteps[0]);
                prevSteps[0] = newSteps[0];
            }
            if (newSteps[1] != prevSteps[1]) {
                // Shoulder uses two mirrored motors for a differential
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
            // newSteps[5] is spare / unused on this hardware

            xSemaphoreGive(motorMutex);

            Serial.printf("Targets → R:%d  SH:%d  GR:%d  FA:%d  WR:%d\n",
                          newSteps[0], newSteps[1], newSteps[2],
                          newSteps[3], newSteps[4]);
        }
    }
}

// =======================================================
// Setup
// =======================================================
void setup() {
    Serial.begin(115200);

    // --- Motor parameters ---
    motorShoulderR.setMaxSpeed(SPEED_STANDARD);
    motorShoulderR.setAcceleration(ACCEL_STANDARD);

    motorShoulderL.setMaxSpeed(SPEED_STANDARD);
    motorShoulderL.setAcceleration(ACCEL_STANDARD);

    motorGeared.setMaxSpeed(SPEED_STANDARD);
    motorGeared.setAcceleration(ACCEL_STANDARD);

    motorForearm.setMaxSpeed(SPEED_STANDARD);
    motorForearm.setAcceleration(ACCEL_STANDARD);

    motorWrist.setMaxSpeed(SPEED_WRIST);
    motorWrist.setAcceleration(ACCEL_WRIST);

    motorRotation.setMaxSpeed(SPEED_ROTATION);
    motorRotation.setAcceleration(ACCEL_ROTATION);

    // --- Mutex ---
    motorMutex = xSemaphoreCreateMutex();

    // --- SPI ---
    spi_handler.initialize();

    // --- FreeRTOS tasks ---
    //
    // Core 0: actuatorTask — blocks on SPI transfer; never busy-waits
    // Core 1: stepperTask  — tight run() loop, highest priority
    //         sensorTask   — sleeps 20 ms between IMU reads
    //
    xTaskCreatePinnedToCore(actuatorTask, "actuatorTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(stepperTask,  "stepperTask",  2048, NULL, 3, NULL, 1);   // priority 3 = highest here
    xTaskCreatePinnedToCore(sensorTask,   "sensorTask",   4096, NULL, 1, NULL, 1);   // priority 1 = lowest here

    Serial.println("System ready.");
}

// -------------------------------------------------------
// Loop — empty; everything runs in FreeRTOS tasks
// -------------------------------------------------------
void loop() {}