// HC-SR04
const int pinLed[] = {
  12,
  3,
  11,
  34,
  13,
  26,
  8,
  32,
  9,
  22,
  10,
  38,
  7,
  28,
  6,
  24,
  5,
  2,
  4,
  36,

};

const int totalLeds = sizeof(pinLed) / sizeof(pinLed[0]);

const int trigPin = 42;
const int echoPin = 44;
float distancia;
bool estadoLed;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  for (int i = 0; i < totalLeds; i++) {
    pinMode(pinLed[i], OUTPUT);
    digitalWrite(pinLed[i], LOW);
  }

  estadoLed = false;
}

void loop() {

  calculo();
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia < 10) {
    estadoLed = !estadoLed;
    Serial.print(estadoLed);
    if (estadoLed == true) {
      for (int i = 0; i < totalLeds; i++) {
        pinMode(pinLed[i], OUTPUT);
        digitalWrite(pinLed[i], HIGH);
        delay(300);
      }
    }

    if (estadoLed == false) {
      for (int i = 0; i < totalLeds; i++) {
        pinMode(pinLed[i], OUTPUT);
        digitalWrite(pinLed[i], LOW);
        delay(300);
      }
    }
  }
  delay(500);
}

void calculo() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  // Lê o retorno
  long duracao = pulseIn(echoPin, HIGH);

  // Converte para centímetros
  distancia = duracao * 0.0343 / 2;
}