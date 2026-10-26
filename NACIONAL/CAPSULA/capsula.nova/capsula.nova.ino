
const int pinLed[] = {
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
  35, 36, 37, 38, 39, 40, 41, 42, 43, 44
};

const int totalLeds = sizeof(pinLed) / sizeof(pinLed[0]);

const int trigPin = 50;
const int echoPin = 51;

float distancia;
bool estadoLed = false;
bool objetoDetectado = false;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  for (int i = 0; i < totalLeds; i++) {
    pinMode(pinLed[i], OUTPUT);
    digitalWrite(pinLed[i], LOW);
  }
}

void loop() {
  bool comandoGrito = false;

  // Recebe comandos enviados pelo Python
  while (Serial.available() > 0) {
    char comando = Serial.read();

    if (comando == 'G') {
      comandoGrito = true;
    }
  }

  // Lê o sensor ultrassônico
  calculo();

  bool objetoPerto = (distancia > 0 && distancia < 10);

  // O sensor aciona apenas uma vez enquanto o objeto
  // permanecer próximo; precisa afastar para rearmar.
  bool acionamentoSensor = objetoPerto && !objetoDetectado;
  objetoDetectado = objetoPerto;

  // O grito ou o sensor podem acionar os LEDs.
  if (comandoGrito || acionamentoSensor) {
    estadoLed = !estadoLed;

    for (int i = 0; i < totalLeds; i++) {
      digitalWrite(pinLed[i], estadoLed ? HIGH : LOW);
      delay(300);
    }
  }

  delay(50);
}

void calculo() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);

  if (duracao == 0) {
    distancia = -1;
  } else {
    distancia = duracao * 0.0343 / 2;
  }
}
