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

void setup() {
  Serial.begin(115200);

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

    String mensagem = Serial.readStringUntil('\n');
    mensagem.trim();

    if (mensagem.length() > 0) {

      Serial.print("Digitado: ");
      Serial.println(mensagem);

      if (SerialBT.connected()) {

        SerialBT.println(mensagem);

        Serial.print("Enviado para ESP B: ");
        Serial.println(mensagem);

      } else {

        Serial.println("ESP B desconectada!");
      }
    }
  }
}