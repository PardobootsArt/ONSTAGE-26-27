#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

const int RX_MEGA = 16;
const int TX_MEGA = 17;

bool sinal2Recebido = false;
bool sinalFinalEnviado = false;

void setup() {
  Serial.begin(115200);
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // ESP C fica como SLAVE. A ESP B conecta pelo MAC.
  SerialBT.begin("ESP_C");

  Serial.println("ESP C INICIADA.");
  Serial.println("Aguardando sinal 2 da ESP B...");
}

void loop() {
  // ESP B -> ESP C
  if (SerialBT.available()) {
    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP B: ");
    Serial.println(mensagem);

    if (mensagem == "2" && !sinal2Recebido) {
      sinal2Recebido = true;

      Serial.println("SINAL 2 RECEBIDO!");
      SerialMega.println("1");
      Serial.println("SINAL 1 ENVIADO PARA O RANDALL!");
    }
  }

  // Randall -> ESP C -> ESP B
  if (sinal2Recebido && SerialMega.available()) {
    String mensagem = SerialMega.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido do Randall: ");
    Serial.println(mensagem);

    if (mensagem == "1" && !sinalFinalEnviado) {
      sinalFinalEnviado = true;

      Serial.println("3 CORES DETECTADAS PELO RANDALL!");

      if (SerialBT.connected()) {
        SerialBT.println("1");
        Serial.println("SINAL 1 ENVIADO PARA ESP B!");
      }
    }
  }
}
