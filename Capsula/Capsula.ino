// HC-SR04
const int pinLed[] = {
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  32,
  33,
  34,
  35,
  36,
  37,
  38,
  39,
  40,
  41,
  42,
  43,
  44,

};

const int totalLeds = sizeof(pinLed) / sizeof(pinLed[0]);

const int trigPin = 50;
const int echoPin = 51;
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