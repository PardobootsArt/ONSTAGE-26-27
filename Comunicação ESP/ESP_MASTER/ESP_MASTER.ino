#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const int IN1 = 14;
const int IN2 = 27;
const int ENA = 12;

// Pino do LED
const int led = 33;

float duration;
float distance;

bool objetoDetectado = false;

// MAC da ESP B
uint8_t macESP_B[] = {
  0x3C,
  0x71,
  0xBF,
  0x45,
  0xC1,
  0xBE
};

const int trigPin = 18;
const int echoPin = 19;

void setup() {
  Serial.begin(115200);

  // Define os pinos do ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Define o pino do LED
  pinMode(led, OUTPUT);

  // Garante que motor e LED iniciem desligados
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  digitalWrite(led, LOW);
  // ESP A como MASTER
  if (!SerialBT.begin("ESP_A", true)) {
    Serial.println("Erro ao iniciar Bluetooth!");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("ESP A iniciada.");
  Serial.println("Tentando conectar na ESP B...");

  if (SerialBT.connect(macESP_B)) {
    Serial.println("ESP B CONECTADA!");
  } else {
    Serial.println("FALHA AO CONECTAR NA ESP B.");
  }
}

void loop() {

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    // Lê o tempo que o eco demorou para voltar
    // Tempo máximo de espera: 30 ms
    duration = pulseIn(echoPin, HIGH, 30000);

    // Se não houver leitura, sai desta repetição do loop
    if (duration == 0) {
      return;
    }

    // Calcula a distância em centímetros
    distance = duration * 0.034 / 2;

    // Executa somente se:
    // 1. O objeto estiver a menos de 10 cm
    // 2. Ainda não tiver sido detectado anteriormente
    if (distance < 10 && objetoDetectado == false) {
    // Marca que o objeto já foi detectado
    // Assim o motor não será acionado novamente
      objetoDetectado = true;

      // Liga o LED e mantém aceso
    analogWrite(led, 255);
    delay(1500);
    // Define o sentido de rotação do motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    // Liga o motor com velocidade 120 (0 a 255)
    analogWrite(ENA, 120);

    // Mantém o motor ligado por 2 segundos
    delay(1950);

    // Desliga o motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);

      if (SerialBT.connected()) {
        Serial.println(1);
        String mensagem = "1";
        SerialBT.println(mensagem);

        Serial.print("Enviado para ESP B: 1");
        Serial.println(mensagem);
      }

      // Pequena pausa antes da próxima leitura
      delay(1000);
    }
}