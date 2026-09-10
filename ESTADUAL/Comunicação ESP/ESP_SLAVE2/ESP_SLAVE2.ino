#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

const int RX_MEGA = 16;
const int TX_MEGA = 17;

bool sinalRecebido = false;

void setup() {
  Serial.begin(115200);
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // Nome usado pela ESP B para encontrar esta ESP.
  SerialBT.begin("ESP_C");

  Serial.println("ESP C iniciada.");
  Serial.println("Aguardando sinal 2 da ESP B...");
}

void loop() {
  if (SerialBT.available()) {
    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP B: ");
    Serial.println(mensagem);

    if (mensagem == "2" && !sinalRecebido) {
      sinalRecebido = true;

      Serial.println("SINAL 2 RECEBIDO!");
      SerialMega.println("1");
      Serial.println("SINAL 1 ENVIADO PARA MEGA 2 / RANDALL!");
    }
  }
}
