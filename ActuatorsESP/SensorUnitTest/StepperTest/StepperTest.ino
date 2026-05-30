#include <AccelStepper.h>

// ============================================================
// HARDWARE CONFIGURATION
// ============================================================
// Driver microstep setting: set your driver DIP switches to 16
// This gives 200 * 16 = 3200 steps/rev at motor shaft
// With 5.18:1 gearbox on stepper3 -> 16,576 steps/rev at output
//
// Recommended drivers: TMC2208 / TMC2209 in StealthChop mode
// for near-silent operation. If using A4988/DRV8825, set to 1/16.
//
// Current: Set via Vref on driver. For NEMA17 2A rated:
//   - Running current: ~1.4A RMS (70%) for silent + thermal comfort
//   - Peak current:     2.0A (for high-torque moments only)
// ============================================================

// ------------------------------------------------------------
// Pin Definitions
// ------------------------------------------------------------
#define STEP_SHOULDER_R   14    // stepper1 - Base Right
#define DIR_SHOULDER_R    15

#define STEP_SHOULDER_L   12    // stepper2 - Base Left  (not connected yet)
#define DIR_SHOULDER_L    13

#define STEP_GEARED 33    // stepper3 - Shoulder (with 5.18:1 gearbox)
#define DIR_GEARED  4

#define STEP_FOREARM  26    // stepper4 - Forearm
#define DIR_FOREARM   25

#define STEP_WRIST    32    // stepper5 - Wrist
#define DIR_WRIST     34

#define STEP_ROTATION 36  // stepper6 - Base Rotation
#define DIR_ROTATION  27

// ------------------------------------------------------------
// Motor Speed & Acceleration Parameters
// ------------------------------------------------------------
// Microstepping = 16  ->  3200 steps/rev (motor shaft)
// Gearbox (stepper3)  ->  3200 * 5.18 = 16576 steps/rev output
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

// ------------------------------------------------------------
// Movement Step Amounts  (in microsteps)
// ------------------------------------------------------------
// 1 full revolution at output = STEPS_PER_REV = 3200 steps
// For arm joints, ~45 degrees = 3200 / 8 = 400 steps
#define STEPS_LARGE         1600     // ~45 deg per command
#define STEPS_SMALL         1200     // ~22 deg per command
#define STEPS_WRIST         50     // wrist fine movement

// ------------------------------------------------------------
// Motor Objects
// ------------------------------------------------------------
AccelStepper motorShoulderR   (AccelStepper::DRIVER, STEP_SHOULDER_R,   DIR_SHOULDER_R);
AccelStepper motorShoulderL   (AccelStepper::DRIVER, STEP_SHOULDER_L,   DIR_SHOULDER_L);
AccelStepper motorGeared(AccelStepper::DRIVER, STEP_GEARED, DIR_GEARED);
AccelStepper motorForearm (AccelStepper::DRIVER, STEP_FOREARM,  DIR_FOREARM);
AccelStepper motorWrist   (AccelStepper::DRIVER, STEP_WRIST,    DIR_WRIST);
AccelStepper motorRotation(AccelStepper::DRIVER, STEP_ROTATION, DIR_ROTATION);

AccelStepper* allMotors[] = {
  &motorShoulderR,
  &motorShoulderL,
  &motorGeared,
  &motorForearm,
  &motorWrist,
  &motorRotation
};
const int MOTOR_COUNT = 6;

// ============================================================
// Setup
// ============================================================
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

  printHelp();
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
  handleSerial();
  runAllMotors();
}

// ============================================================
// Serial Command Handler
// ============================================================
void handleSerial() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  switch (cmd) {

    // --- Base Rotation (stepper6) ---
    case 'l':
      motorRotation.move(STEPS_LARGE);
      Serial.println("BASE → Left");
      break;

    case 'r':
      motorRotation.move(-STEPS_LARGE);
      Serial.println("BASE → Right");
      break;

    // --- Gearbox Joint (stepper3) ---
    case 'u':
      motorGeared.move(STEPS_LARGE * 3);
      Serial.println("SHOULDER → Up");
      break;

    case 'd':
      motorGeared.move(-STEPS_LARGE * 3);
      Serial.println("SHOULDER → Down");
      break;

    // --- Forearm (stepper4) ---
    case 'f':
      motorForearm.move(STEPS_LARGE);
      Serial.println("FOREARM → Forward");
      break;

    case 'b':
      motorForearm.move(-STEPS_LARGE);
      Serial.println("FOREARM → Back");
      break;

    // --- Wrist (stepper5) ---
    case 'h':
      motorWrist.move(STEPS_WRIST);
      Serial.println("WRIST → Up");
      break;

    case 'n':
      motorWrist.move(-STEPS_WRIST);
      Serial.println("WRIST → Down");
      break;

    // --- Shoulder Right / Left differential (stepper1 & stepper2) ---
    case 't':
      motorShoulderR.move(STEPS_LARGE);
      motorShoulderL.move(-STEPS_LARGE);
      Serial.println("BASE DIFF → Tilt Right");
      break;

    case 'y':
      motorShoulderR.move(-STEPS_LARGE);
      motorShoulderL.move(STEPS_LARGE);
      Serial.println("BASE DIFF → Tilt Left");
      break;

    // --- Preset Position 1 (pickup pose) ---
    // case 'p':
    //   runSequence_Pickup();
    //   break;

    // // --- Preset Position 2 (home / return) ---
    // case 'z':
    //   runSequence_Home();
    //   break;

    // --- Stop All ---
    case 's':
      stopAll();
      Serial.println("STOP ALL");
      break;

    case '?':
      printHelp();
      break;

    default:
      break;
  }
}

// ============================================================
// Run All Motors (non-blocking)
// ============================================================
void runAllMotors() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    allMotors[i]->run();
  }
}

// ============================================================
// Stop All Motors
// ============================================================
void stopAll() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    allMotors[i]->stop();
  }
}

// ============================================================
// Blocking move helper — runs one motor to relative position
// ============================================================
void moveBlocking(AccelStepper& motor, long steps, unsigned int waitMs = 300) {
  motor.move(steps);
  motor.runToPosition();
  delay(waitMs);
}

// ============================================================
// Preset Sequence: Pickup Position
// ============================================================
// void runSequence_Pickup() {
//   Serial.println("SEQ → Pickup Start");

//   moveBlocking(motorForearm,   STEPS_SMALL,       300);
//   moveBlocking(motorShoulder, -STEPS_LARGE * 2,   500);
//   moveBlocking(motorShoulderR,     STEPS_LARGE,        300);
//   moveBlocking(motorBaseR,     STEPS_LARGE,        300);
//   moveBlocking(motorWrist,     STEPS_WRIST,        300);

//   Serial.println("SEQ → Pickup Done");
// }

// ============================================================
// Preset Sequence: Home Position
// ============================================================
// void runSequence_Home() {
//   Serial.println("SEQ → Home Start");

//   moveBlocking(motorShoulderR,    -STEPS_LARGE,        300);
//   moveBlocking(motorBaseR,    -STEPS_LARGE,        300);
//   moveBlocking(motorShoulder,  STEPS_LARGE * 2,    500);
//   moveBlocking(motorWrist,    -STEPS_WRIST,        300);
//   moveBlocking(motorForearm,  -STEPS_SMALL,        300);

//   Serial.println("SEQ → Home Done");
// }

// ============================================================
// Help Text
// ============================================================
void printHelp() {
  Serial.println(F("==========================================="));
  Serial.println(F(" 6-AXIS ARM CONTROLLER  |  16-microstep  "));
  Serial.println(F("==========================================="));
  Serial.println(F(" l / r  → Base Rotation  (left / right)  "));
  Serial.println(F(" u / d  → Shoulder       (up / down)     "));
  Serial.println(F(" f / b  → Forearm        (fwd / back)    "));
  Serial.println(F(" h / n  → Wrist          (up / down)     "));
  Serial.println(F(" t / y  → Base Diff      (tilt R / L)    "));
  Serial.println(F(" p      → Preset: Pickup position        "));
  Serial.println(F(" z      → Preset: Home position          "));
  Serial.println(F(" s      → Stop all motors                "));
  Serial.println(F(" ?      → Show this help                 "));
  Serial.println(F("==========================================="));
}