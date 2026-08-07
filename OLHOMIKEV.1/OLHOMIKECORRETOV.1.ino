#include <Servo.h>

Servo vertical;    // Porta 9
Servo horizontal;  // Porta 10

void setup() {
  vertical.attach(9); //olhara para cima e baixo
  horizontal.attach(10); // olhar para os lados

  vertical.write(90);
  horizontal.write(90);
}

void loop() {

  // Olha para a direita
  horizontal.write(180);
  delay(1000);

  // Olha para a esquerda
  horizontal.write(0);
  delay(1000);

  // Volta para o centro
  horizontal.write(90);
  delay(1000);

  // Olha para cima
  vertical.write(180);
  delay(1000);

  // Olha para baixo
  vertical.write(0);
  delay(1000);

  // Centro
  vertical.write(90);
  delay(1000);
}