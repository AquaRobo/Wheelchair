#include "/home/omar/mashro3_final/ESP/headers/SPIHandler.h"
#include "/home/omar/mashro3_final/ESP/services/SPIHandler.cpp"
#include <AccelStepper.h>
#include <stdlib.h>

// -------------------------------------------------------
// SPI
// -------------------------------------------------------
SPIHandler spi_handler(40, 4);

// -------------------------------------------------------
// Pin Definitions
// -------------------------------------------------------
#define STEP_PIN_1  14
#define STEP_PIN_2  27
#define STEP_PIN_3  26
#define STEP_PIN_4  25
#define STEP_PIN_5  33
#define STEP_PIN_6  32

#define DIR_PIN_1   12
#define DIR_PIN_2   13
#define DIR_PIN_3   15
#define DIR_PIN_4   2
#define DIR_PIN_5   4
#define DIR_PIN_6   5

// -------------------------------------------------------
// Stepper Motors
// -------------------------------------------------------
AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);
AccelStepper stepper3(AccelStepper::DRIVER, STEP_PIN_3, DIR_PIN_3);
AccelStepper stepper4(AccelStepper::DRIVER, STEP_PIN_4, DIR_PIN_4);
AccelStepper stepper5(AccelStepper::DRIVER, STEP_PIN_5, DIR_PIN_5);
AccelStepper stepper6(AccelStepper::DRIVER, STEP_PIN_6, DIR_PIN_6);

AccelStepper* steppers[6] = {
    &stepper1, &stepper2, &stepper3,
    &stepper4, &stepper5, &stepper6
};

// -------------------------------------------------------
// Mutex — protects spi_handler across both cores
// -------------------------------------------------------
SemaphoreHandle_t spiMutex;

// -------------------------------------------------------
// Globals
// -------------------------------------------------------
char cmd;
float v1, v2;

// -------------------------------------------------------
// Sensor globals
// -------------------------------------------------------
float orientation_x, orientation_y, orientation_z, orientation_w;
float linear_acc_x,  linear_acc_y,  linear_acc_z;
float angular_vel_x, angular_vel_y, angular_vel_z;

// -------------------------------------------------------
// Sensor Task — Core 1
// Fills TX buffer with IMU data at 50Hz
// -------------------------------------------------------
void sensorTask(void* arg) {
    while (true) {
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

        // protect spi_handler access
        if (xSemaphoreTake(spiMutex, portMAX_DELAY)) {
            spi_handler.fillTXBuffer(imu_payload, sizeof(imu_payload));
            xSemaphoreGive(spiMutex);
        }

        vTaskDelay(20); // 50 Hz
    }
}

// -------------------------------------------------------
// Actuator Task — Core 0
// Handles SPI transfer, command parsing, stepper control
// -------------------------------------------------------
void actuatorTask(void* arg) {
    while (true) {

        // protect spi_handler access
        if (xSemaphoreTake(spiMutex, portMAX_DELAY)) {
            spi_handler.transferBuffers();
            xSemaphoreGive(spiMutex);
        }

        auto actuators = spi_handler.getRXData();

        if (actuators.size() >= 1) {
            cmd = actuators[0];

            // 'w' command — receives 2 floats
            if (cmd == 'w' && actuators.size() >= 9) {
                memcpy(&v1, &actuators[4], 4);
                memcpy(&v2, &actuators[8], 4);
                Serial.printf("Received: %c, %.2f, %.2f\n", cmd, v1, v2);
            }

            // 's' command — receives 6 ints (steps per motor)
            else if (cmd == 's' && actuators.size() >= 28) {
                int32_t newSteps[6];
                memcpy(&newSteps[0], &actuators[4],  4);
                memcpy(&newSteps[1], &actuators[8],  4);
                memcpy(&newSteps[2], &actuators[12], 4);
                memcpy(&newSteps[3], &actuators[16], 4);
                memcpy(&newSteps[4], &actuators[20], 4);
                memcpy(&newSteps[5], &actuators[24], 4);

                for (int i = 0; i < 6; i++) {
                    steppers[i]->moveTo(newSteps[i]);
                }

                Serial.printf("Targets → M1:%d M2:%d M3:%d M4:%d M5:%d M6:%d\n",
                              newSteps[0], newSteps[1], newSteps[2],
                              newSteps[3], newSteps[4], newSteps[5]);
            }
        }

        // keep steppers moving toward target
        for (int i = 0; i < 6; i++) {
            steppers[i]->run();
        }

        taskYIELD(); // yield without sleeping for faster stepper pulses
    }
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {
    Serial.begin(115200);

    // create mutex before starting tasks
    spiMutex = xSemaphoreCreateMutex();

    // configure each stepper
    for (int i = 0; i < 6; i++) {
        steppers[i]->setMaxSpeed(500);
        steppers[i]->setAcceleration(100);
    }

    spi_handler.initialize();

    xTaskCreatePinnedToCore(actuatorTask, "ActuatorTask", 4096, NULL, 2, NULL, 0); // Core 0
    xTaskCreatePinnedToCore(sensorTask,   "SensorTask",   4096, NULL, 1, NULL, 1); // Core 1

    Serial.println("SPIHandler Initialized.");
}

// -------------------------------------------------------
// Loop — empty, everything runs in FreeRTOS tasks
// -------------------------------------------------------
void loop() {}