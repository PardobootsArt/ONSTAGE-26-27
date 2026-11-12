#include <Servo.h>
Servo direito;
Servo esquerdo;
Servo olhoDirC;
Servo olhoDirB;
Servo olhoEsqC;
Servo olhoEsqB;

#define pescoco1 8
#define pescoco2 9

void palpebras(int abrir, int fechar) {
  if (abrir > 0 && fechar < 0) {
    olhoEsqC.write(25);
    delay(25);
    olhoEsqB.write(55);
    delay(50);
    olhoDirB.write(10);
    delay(25);
    olhoDirC.write(0);
  }
  if (abrir < 0 && fechar > 0) {
    olhoEsqC.write(65);
    delay(25);
    olhoEsqB.write(5);
    delay(50);
    olhoDirC.write(90);
    delay(25);
    olhoDirB.write(90);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  direito.attach(12);//baixo=180 alto=0
  esquerdo.attach(13);//baixo=0 alto=180

  olhoEsqC.attach(3);
  olhoEsqB.attach(4);

  olhoDirC.attach(5);
  olhoDirB.attach(6);

  pinMode(pescoco1, OUTPUT);
  pinMode(pescoco2, OUTPUT);
  delay(1500);
  palpebras(1,-1);
}

void loop() {
  //direita
  digitalWrite(pescoco1, HIGH);
  digitalWrite(pescoco2, LOW);
  delay(450);
  digitalWrite(pescoco1, LOW);
  digitalWrite(pescoco2, LOW);
  direito.write(0);
  esquerdo.write(180);
  palpebras(-1,1);
  delay(1000);
  palpebras(1,-1);
  //esquerda
  digitalWrite(pescoco2, HIGH);
  digitalWrite(pescoco1, LOW);
  delay(450);
  digitalWrite(pescoco1, LOW);
  digitalWrite(pescoco2, LOW);
  delay(150);
  direito.write(180);
  esquerdo.write(0);
  delay(1000);

}
