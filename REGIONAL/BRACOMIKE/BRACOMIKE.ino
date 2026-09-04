#include <Servo.h>

Servo bracoEsquerdo;
Servo bracoDireito;

void setup() {

  bracoEsquerdo.attach(8);
  bracoDireito.attach(7);

  // Posição inicial
  bracoEsquerdo.write(0);
  bracoDireito.write(0);

  delay(2000);

  // Levanta os braços
  bracoEsquerdo.write(180);
  delay (100);
  bracoDireito.write(90);

  // Mantém levantados por 5 segundos
  delay(5000);

  // Abaixa os braços
  bracoEsquerdo.write(0);
  delay (100);
  bracoDireito.write(0);
}

void loop() {
  
}
