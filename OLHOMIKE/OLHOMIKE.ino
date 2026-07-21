#include <Servo.h>

Servo servoA;
Servo servoB;

void setup() {
  servoA.attach(9);
  servoB.attach(10);
}

void loop() {
  // Vai de 0 a 180
  for (int n = 0; n <= 180; n++) {
    servoA.write(n);
    servoB.write(n);
    delay(10);
  }

  // Volta de 180 a 0
  for (int n = 180; n >= 0; n--) {
    servoA.write(n);
    servoB.write(n);
    delay(10);
  }
}