#include <Servo.h> //biblioteca

Servo direito;
Servo esquerdo;

void setup() {
  // put your setup code here, to run once:

direito.attach(11); //porta 12
esquerdo.attach(13); //porta 13

esquerdo.write(180); 
direito.write(0);
delay(500);
direito.write(90); 
esquerdo.write(120);
delay(500);
direito.write(90);
esquerdo.write(120);
delay(500);




}

void loop() {
  // put your main code here, to run repeatedly:



}