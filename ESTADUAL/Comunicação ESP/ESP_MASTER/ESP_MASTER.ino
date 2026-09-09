#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// ================= BLUETOOTH =================
uint8_t macESP_B[] = {
  0x3C, 
  0x71, 
  0xBF, 
  0x45, 
  0xC1, 
  0xBE
};

// ================= ULTRASSOM =================
const int trigPin = 18;
const int echoPin = 19;

// ================= MOTOR DA PORTA =================
const int IN1 = 12;
const int IN2 = 13;
const int ENA = 11;

// ================= FITA LED =================
const int led = 33;

bool objetoDetectado = false;
bool portaAberta = false;

long lerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);

  if (duracao == 0) return 0;

  return duracao * 0.034 / 2;
}

void abrirPorta() {
  Serial.println("Abrindo porta...");

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);

  delay(1000);

  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  portaAberta = true;
  Serial.println("Porta aberta!");
}

void enviarSinal1() {
  if (SerialBT.connected()) {
    SerialBT.println("1");
    Serial.println("SINAL 1 ENVIADO PARA ESP B!");
  } else {
    Serial.println("ESP B nao esta conectado!");
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

  analogWrite(ENA, 0);

  SerialBT.begin("ESP_A", true);

  Serial.println("ESP A iniciada.");
  Serial.println("Tentando conectar na ESP B...");

  if (SerialBT.connect(macESP_B)) {
    Serial.println("ESP A conectada na ESP B!");
  } else {
    Serial.println("Falha ao conectar na ESP B.");
  }
}

void loop() {
  if (!SerialBT.connected()) {
    static unsigned long ultimaTentativa = 0;

    if (millis() - ultimaTentativa > 5000) {
      ultimaTentativa = millis();
      Serial.println("Tentando reconectar na ESP B...");
      SerialBT.connect(macESP_B);
    }
  }

  long distancia = lerDistancia();

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Detecta objeto somente uma vez
  if (distancia > 0 && distancia < 10 && !objetoDetectado) {
    objetoDetectado = true;

    digitalWrite(led, HIGH);
    abrirPorta();

    // Sinal enviado 1 segundo depois de a porta abrir
    delay(1000);
    enviarSinal1();
  }

  // Permite uma nova detecção quando o objeto sair
  if (distancia > 15) {
    objetoDetectado = false;
  }

  delay(100);
}
