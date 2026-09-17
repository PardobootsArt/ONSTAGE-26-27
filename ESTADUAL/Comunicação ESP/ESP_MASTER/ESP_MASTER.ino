#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// ESP B
uint8_t macESP_B[] = {0x3C, 0x71, 0xBF, 0x45, 0xC1, 0xBE};

// Ultrassonico
const int trigPin = 18;
const int echoPin = 19;

// Motor da porta
const int IN1 = 2;
const int IN2 = 15;
const int ENA = 4;

// Fita LED
const int led = 33;

bool objetoDetectado = false;
bool portaAberta = false;
unsigned long ultimaTentativaBT = 0;

long lerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  if (duracao == 0) return -1;
  return duracao * 0.034 / 2;
}

void abrirPorta() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
  delay(1000);
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  portaAberta = true;
  Serial.println("PORTA ABERTA!");
}

void enviarSinal1() {
  if (SerialBT.connected()) {
    SerialBT.println("1");
    Serial.println("SINAL 1 ENVIADO PARA ESP B!");
  } else {
    Serial.println("ESP B ainda nao conectada.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(led, OUTPUT);

  digitalWrite(led, LOW);

  SerialBT.begin("ESP_A", true);
  Serial.println("ESP A iniciada.");
  Serial.println("Ultrassonico funcionando enquanto procura a ESP B...");
}

void loop() {
  // Tenta conectar sem travar o inicio do ultrassonico.
  if (!SerialBT.connected() && millis() - ultimaTentativaBT >= 5000) {
    ultimaTentativaBT = millis();
    Serial.println("Tentando conectar na ESP B...");
    if (SerialBT.connect(macESP_B)) {
      Serial.println("ESP B conectada!");
    } else {
      Serial.println("Falha ao conectar na ESP B.");
    }
  }

  long distancia = lerDistancia();

  if (distancia > 0) {
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  if (distancia > 0 && distancia < 10 && !objetoDetectado) {
    objetoDetectado = true;
    Serial.println("OBJETO DETECTADO!");

    digitalWrite(led, HIGH);
    abrirPorta();
    delay(1000);
    enviarSinal1();
  }

  if (distancia > 15) {
    objetoDetectado = false;
  }

  delay(100);
}
