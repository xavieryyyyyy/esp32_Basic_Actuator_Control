# Laboratory Activity 6: Basic Actuator Control

This repository contains my implementation of **Laboratory Activity 6: Basic Actuator Control** using an **ESP32-S3-N16R8**, a **28BYJ-48 5 V stepper motor**, and a **ULN2003 stepper motor driver board**.

The original Example 7 uses a brushed DC motor driver with `AIN1`, `AIN2`, `nSLEEP`, and `VM`. Since the available actuator was a 28BYJ-48 stepper motor with its paired ULN2003 driver, the activity was adapted while preserving the required **button-controlled run/stop behavior**.

## Objective

The activity demonstrates basic actuator control using a push button.

- **Button released:** motor is stopped and the driver outputs are de-energized.
- **Button pressed and held:** stepper motor rotates continuously.
- **Button released after running:** motor stops and the ULN2003 indicator LEDs turn off.
- **Button held during reset/startup:** the motor pauses during reset and begins rotating again after the ESP32 finishes initialization.

## Hardware Used

| Component | Actual Hardware |
|---|---|
| Microcontroller | ESP32-S3-N16R8 |
| Motor | 28BYJ-48 5 V unipolar stepper motor |
| Motor driver | Generic ULN2003 stepper driver board |
| Driver IC | ULN2003AN |
| Input | Momentary push button |
| Motor supply | 5 V from Arduino board |
| Motor connection | 5-pin connector to ULN2003 board |

## Motor Specifications

The motor is physically marked **28BYJ-48 5V**. No manufacturer name is printed on the available motor, so the linked 28BYJ-48 datasheet is used as the model reference.

| Specification | Value |
|---|---:|
| Rated voltage | 5 V DC |
| Number of phases | 4 |
| Coil type | Unipolar |
| DC resistance | 50 Ω ±7% at 25 °C |
| Gear reduction | 1/64 |
| Stride angle | 5.625° / 64 |
| Rated/reference frequency | 100 Hz |

### Current / Stall-Current Note

The 28BYJ-48 datasheet does **not specify a brushed-DC-style stall current**. Since this activity uses a stepper motor substitution, that parameter is not directly applicable.

Using the nominal winding resistance:

`I = V / R = 5 V / 50 Ω ≈ 0.10 A`

This is approximately **100 mA per energized winding** before driver voltage drop. The full-step sequence used by this program energizes two windings at a time, so the nominal winding-current total is approximately **200 mA**. This calculated value is provided for electrical context and is **not claimed as a datasheet stall-current rating**.

## Driver Specifications

The breakout board has no separate model marking, but its IC is marked **ULN2003AN**.

| Specification | Value |
|---|---:|
| Driver type | 7-channel Darlington transistor array |
| Driver IC | ULN2003AN |
| Maximum output voltage of IC | 50 V |
| Rated collector current per output | 500 mA |
| Supply used in this experiment | 5 V |
| Motor control inputs used | IN1, IN2, IN3, IN4 |

## Datasheet References

- **28BYJ-48 5 V Stepper Motor:** `ADD_28BYJ48_DATASHEET_LINK_HERE`
- **ULN2003AN / ULN2003A:** `ADD_ULN2003AN_DATASHEET_LINK_HERE`

> The 28BYJ-48 motor has no manufacturer name printed on its casing. The linked document should therefore be described as the reference datasheet for the same motor model rather than claiming a manufacturer that is not printed on the actual hardware.

## Wiring

### ESP32-S3 to ULN2003

| ESP32-S3 | ULN2003 |
|---|---|
| GPIO 4 | IN1 |
| GPIO 5 | IN2 |
| GPIO 6 | IN3 |
| GPIO 7 | IN4 |
| GND | Common GND |

### Push Button

| Connection | Destination |
|---|---|
| GPIO 15 | One side of push button |
| GND | Other side of push button |

The button uses the ESP32 internal pull-up resistor with `INPUT_PULLUP`. Therefore:

| Button State | GPIO Reading |
|---|---|
| Released | HIGH |
| Pressed | LOW |

### Motor Power

| Source | Destination |
|---|---|
| Arduino 5 V | ULN2003 `+` |
| Arduino GND | ULN2003 `- / GND` |
| ESP32 GND | Same common ground |
| 28BYJ-48 plug | ULN2003 5-pin motor socket |

The Arduino board is used only as the **5 V motor supply**. Motor control is performed by the ESP32-S3.

## Hardware Substitution from Example 7

The original lecture example is based on a brushed DC motor driver. The available hardware required the following functional substitutions:

| Example 7 Function | This Implementation |
|---|---|
| AIN1 / AIN2 | IN1–IN4 stepper phase controls |
| nSLEEP LOW | IN1–IN4 all LOW using `motorOff()` |
| nSLEEP HIGH / enabled | Step sequence active while the button is held |
| VM | 5 V motor supply connected to ULN2003 |
| Common ground | ESP32, ULN2003, and Arduino supply share GND |
| DC motor run | 28BYJ-48 continuously steps |
| DC motor coast | Stepper coils are de-energized and motor stops |

The ULN2003 does **not** contain a true `nSLEEP` input. Setting IN1–IN4 LOW is used only as the practical motor-disable equivalent for this implementation; it is not a hardware sleep mode.

## Roles of the Reference Driver Signals

### AIN1 and AIN2

In the reference H-bridge driver, `AIN1` and `AIN2` control the drive state and direction of Motor A.

The ULN2003 implementation instead requires four inputs (`IN1`–`IN4`) because the 28BYJ-48 is a four-phase unipolar stepper motor.

### nSLEEP

`nSLEEP` is used by the reference driver to enable or place the motor driver into a low-power disabled state.

The ULN2003 board does not have `nSLEEP`. In this project, the motor is disabled by setting all four control inputs LOW:

```cpp
void motorOff() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
}
```

This de-energizes the stepper motor coils.

### VM

`VM` is the motor power-supply input of the reference driver.

For this implementation, the functional equivalent is the **5 V supply connected to the ULN2003 motor-driver board**.

### Common Ground

A common ground provides the same voltage reference for the ESP32 control signals and the ULN2003 driver.

The following grounds are connected together:

- ESP32-S3 GND
- ULN2003 GND
- Arduino GND used by the 5 V motor supply

## Source Code

The complete Arduino sketch is located at:

`src/lab6_basic_actuator_control.ino`

```cpp
#include <Arduino.h>

const uint8_t BUTTON_PIN = 15;

const uint8_t MOTOR_IN1 = 4;
const uint8_t MOTOR_IN2 = 5;
const uint8_t MOTOR_IN3 = 6;
const uint8_t MOTOR_IN4 = 7;

int stepNumber = 0;

void motorOff() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
}

void setup() {
  // Motor control pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);

  // Button input
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Motor starts off
  motorOff();
}

void loop() {
  const bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (buttonPressed) {

    if (stepNumber == 0) {
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, HIGH);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, LOW);
    }
    else if (stepNumber == 1) {
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, HIGH);
      digitalWrite(MOTOR_IN3, HIGH);
      digitalWrite(MOTOR_IN4, LOW);
    }
    else if (stepNumber == 2) {
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, LOW);
      digitalWrite(MOTOR_IN3, HIGH);
      digitalWrite(MOTOR_IN4, HIGH);
    }
    else if (stepNumber == 3) {
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, HIGH);
    }

    stepNumber++;

    if (stepNumber > 3) {
      stepNumber = 0;
    }

    delay(10);
  }
  else {
    motorOff();
  }
}
```

## How the Program Works

The push button is configured using `INPUT_PULLUP`, so pressing the button produces a LOW input.

While the button is held, the program repeatedly applies this four-step sequence:

| Step | IN1 | IN2 | IN3 | IN4 |
|---:|---:|---:|---:|---:|
| 1 | HIGH | HIGH | LOW | LOW |
| 2 | LOW | HIGH | HIGH | LOW |
| 3 | LOW | LOW | HIGH | HIGH |
| 4 | HIGH | LOW | LOW | HIGH |

A `10 ms` delay is used between steps. This sequence produced stable clockwise rotation during the actual hardware test.

When the button is released, `motorOff()` sets IN1–IN4 LOW and de-energizes the motor.

## Observed Results

| Test | Observed Behavior | Result |
|---|---|---|
| Power on with button released | Motor remained stopped and ULN2003 LEDs were OFF | PASS |
| Button pressed and held | Motor rotated continuously | PASS |
| Rotation quality | Smooth rotation | PASS |
| Rotation direction | Clockwise | OBSERVED |
| Button released after running | Motor stopped immediately | PASS |
| Driver LEDs after release | All ULN2003 LEDs turned OFF | PASS |
| Button held during ESP32 reset | Motor stopped during reset and resumed after startup while button remained held | PASS |
| Motor supply removed | Motor did not run | EXPECTED |

The observed results verify the required **hold-to-run and release-to-stop behavior**.

## Wiring Evidence

Place the final wiring photo in:

`media/wiring.jpg`

Then it will appear here:

![Complete Lab 6 wiring](media/wiring.jpg)

## Actual Demo Video

Place the actual hardware demonstration in:

`media/demo.mp4`

The video should show:

1. Motor stopped while the button is released.
2. Button pressed and held.
3. Motor rotating clockwise.
4. ULN2003 indicator LEDs sequencing while the motor operates.
5. Button released.
6. Motor stopping and the ULN2003 LEDs turning off.
7. Optional reset/startup test while the button remains held.

[View the actual Lab 6 demonstration](media/demo.mp4)

## Optional Potentiometer PWM Extension

The optional PWM experiment was **not performed**.

The original optional task is intended for a brushed DC motor, where PWM duty cycle directly controls average motor voltage and the onset of rotation can be measured.

The substituted 28BYJ-48 is a stepper motor. Its rotation is controlled primarily by the **step sequence and step interval**, rather than by the same PWM-duty method used for a brushed DC motor.

## Conclusion

The actuator-control objective was successfully demonstrated using the available ESP32-S3, 28BYJ-48 5 V stepper motor, and ULN2003AN-based driver board.

Holding the push button caused the motor to rotate continuously clockwise. Releasing the button stopped the motor and de-energized all four driver inputs. The motor also resumed operation after an ESP32 reset when the button remained held, preserving the level-controlled behavior demonstrated by Example 7.

Although the available hardware differs from the reference brushed-DC H-bridge setup, the substitution was documented explicitly and the required button-controlled actuator behavior was successfully verified.
