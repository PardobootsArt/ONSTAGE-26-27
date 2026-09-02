#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

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

float duration;
float distance;

bool objetoDetectado = false;

void setup() {
  Serial.begin(115200);

  // Define os pinos do ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

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

  if (Serial.available()) {

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

      if (SerialBT.connected()) {
        Serial.println(1);
        String mensagem = "detectado";
        SerialBT.println(mensagem);

        Serial.print("Enviado para ESP B: ");
        Serial.println(mensagem);
        // Marca que o objeto já foi detectado
        // Assim o motor não será acionado novamente
      }

      // Pequena pausa antes da próxima leitura
      delay(100);
    }
  }
}