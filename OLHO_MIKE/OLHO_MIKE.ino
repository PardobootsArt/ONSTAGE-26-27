#include <Servo.h> //biblioteca

Servo horizontal;
Servo vertical;

void setup() {
  // put your setup code here, to run once:

horizontal.attach(10); //porta 10
vertical.attach(9); //porta 9

vertical.write(45);
horizontal.write(0);
delay(500);
horizontal.write(90);
delay(2000);

}

void loop() {
  // put your main code here, to run repeatedly:

vertical.write(45);
horizontal.write(45);
delay(500);
horizontal.write(0);
delay(500);
horizontal.write(90);
delay(500);
horizontal.write(45);
delay(500);
vertical.write(120);
delay(500);
vertical.write(0);
delay(500);
vertical.write(35);


}