#include <Servo.h>

Servo servoA;
Servo servoB;

void setup() {
  servoA.attach(9);
  servoB.attach(10);
}

void loop() {
  // Servo A: 0 -> 180
  // Servo B: 180 -> 0
  for (int n = 0; n <= 180; n++) {
    servoA.write(n);
    servoB.write(180 - n);
    delay(100);
  }

  // Volta
  for (int n = 180; n >= 0; n--) {
    servoA.write(n);
    servoB.write(180 - n);
    delay(100);
  }
}  