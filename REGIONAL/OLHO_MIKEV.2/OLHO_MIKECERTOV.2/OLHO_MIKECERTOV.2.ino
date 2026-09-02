#include <Servo.h>

Servo horizontal;
Servo vertical;

void setup() {
  // put your setup code here, to run once:
  horizontal.attach(10);  //porta 10
  vertical.attach(9);

  olho(45, 50); // centro
  olho(0,50); //cima
  olho(0,0); // superior esquerdo
  olho(45,0);// esquerdo
  olho(90,0);//inferior esquerdo
  olho(90,50); //baixo
  olho(90,90); // inferior direito
  olho(45,90); //direito
  olho(0,90); //superior direito
}

void loop() {
  // put your main code here, to run repeatedly:
}

void olho(int vertical2, int horizontal2) {
  //porta 9
  vertical.write(vertical2);
  horizontal.write(horizontal2);
  delay(1000);
}
