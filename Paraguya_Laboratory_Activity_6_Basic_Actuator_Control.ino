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