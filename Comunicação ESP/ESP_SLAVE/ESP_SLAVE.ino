#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {

  // Monitor Serial
  Serial.begin(115200);

  // Comunicação ESP B → Arduino Mega
  // RX = GPIO 16
  // TX = GPIO 17
  Serial2.begin(9600, SERIAL_8N1, 16, 17);


  // ESP B como receptora
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

  // Verifica se chegou alguma mensagem da ESP A
  if (SerialBT.available()) {

    // Lê a mensagem até a quebra de linha
    String mensagem = SerialBT.readStringUntil('\n');

    // Remove espaços e caracteres desnecessários
    mensagem.trim();

    // Verifica se realmente recebeu alguma coisa
    if (mensagem.length() > 0) {

      Serial.print("Recebido da ESP A: ");
      Serial.println(mensagem);

      // Se a ESP A mandar "1"
      if (mensagem == "1") {

        Serial.println("SINAL RECEBIDO!");


        // Manda automaticamente "1" para o Mega
        Serial2.println("1");

        Serial.println("Enviado para o Arduino Mega: 1");
      }
    }
  }
}