#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const int LED = 2;
void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);


  if (!SerialBT.begin("ESP_B")) {
    Serial.println("Erro ao iniciar Bluetooth!");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("ESP B iniciada.");
  Serial.println("Bluetooth iniciado.");
  Serial.println("Nome Bluetooth: ESP_B");
  Serial.println("Aguardando ESP A...");
}

void loop() {

  if (SerialBT.available()) {

    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    if (mensagem.length() > 0) {

      Serial.print("Recebido da ESP A: ");
      Serial.println(mensagem);

      if (mensagem == "1") {
        Serial.println("SINAL 1 RECEBIDO!");
        digitalWrite(LED, HIGH);
      }
        if (mensagem == "2"){

          Serial.print("SINAL 2 RECEBIDO!");
          digitalWrite(LED, LOW);
        }
    }
  }
}