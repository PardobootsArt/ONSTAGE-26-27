#define pescoco1 10
#define pescoco2 8
#define enbPesc 11
#define yPesc 2
#define bPesc 9
volatile long degrees;

#include <Servo.h>
Servo direito;
Servo esquerdo;
Servo olhoDirC;
Servo olhoDirB;
Servo olhoEsqC;
Servo olhoEsqB;

void countDegrees() {
  if (digitalRead(yPesc) == digitalRead(bPesc)) {
    degrees++;
  } else {
    degrees--;
  }
}

void girarPescoco(int speed, int anguloo) {
  analogWrite(enbPesc, speed);

  if (anguloo > degrees) {
    while (degrees < anguloo) {
      Serial.print("graus: ");
      Serial.println(degrees);
      digitalWrite(pescoco2, HIGH);
      digitalWrite(pescoco1, LOW);
    }

    digitalWrite(pescoco1, LOW);
    digitalWrite(pescoco2, LOW);
  }

  if (anguloo < degrees) {
    while (degrees > anguloo) {
      Serial.print("graus: ");
      Serial.println(degrees);
      digitalWrite(pescoco2, LOW);
      digitalWrite(pescoco1, HIGH);
    }

    digitalWrite(pescoco1, LOW);
    digitalWrite(pescoco2, LOW);
  }
}

void palpebras(int abrir, int fechar) {
  if (abrir > 0 && fechar < 0) {
    olhoEsqC.write(25);
    delay(25);
    olhoEsqB.write(40);
    delay(50);
    olhoDirB.write(10);
    delay(25);
    olhoDirC.write(60);
  }
  if (abrir < 0 && fechar > 0) {
    olhoEsqC.write(90);
    delay(25);
    olhoEsqB.write(0);
    delay(50);
    olhoDirC.write(10);
    delay(25);
    olhoDirB.write(70);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  direito.attach(12);
  esquerdo.attach(13);

  olhoEsqC.attach(3);
  olhoEsqB.attach(4);

  olhoDirC.attach(5);
  olhoDirB.attach(6);
  palpebras(1, -1);

  degrees = 0;
  attachInterrupt(
    digitalPinToInterrupt(yPesc),
    countDegrees,
    CHANGE);

}

void loop() {
  delay(2000);
  //cima
  direito.write(0);
  esquerdo.write(180);
  delay(1500);
  //baixo
  direito.write(180);
  esquerdo.write(0);
}
