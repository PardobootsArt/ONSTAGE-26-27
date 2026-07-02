#include <Servo.h>

Servo servoA;
Servo servoB;

void setup() {
  // put your setup code here, to run once:
  servoA.attach(9);
  servoB.attach(10);
}

void loop() {
  // put your main code here, to run repeatedly:
  int n;
  for(n=0;n<180;n++){
    servoA.write(n);
    if(n==0||n==45||n==90||n==135||n==179){
      servoB.write(n);}
    delay(10);
  }
  delay(100);
  
  for(n=180;n>0;n--){
    servoA.write(n);
    if(n==45||n==90||n==135){
      servoB.write(n);}
    delay(10);}
  delay(100);
}