#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// ================= SERIAL COM MEGA 2 =================
// ESP C TX GPIO17 -> Mega 2 RX1 (19)
// ESP C RX GPIO16 <- Mega 2 TX1 (18)
HardwareSerial SerialMega(2);

const int RX_MEGA = 16;
const int TX_MEGA = 17;

bool sinalRecebido = false;

void setup() {
  Serial.begin(115200);

  // Serial com o segundo Mega
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // ESP C funciona como SLAVE.
  // A ESP B entra como MASTER e procura pelo nome ESP_C.
  SerialBT.begin("ESP_C");

  Serial.println("ESP C iniciada.");
  Serial.println("Aguardando sinal da ESP B...");
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
      Serial.println("Enviando sinal 1 para MEGA 2...");

      // Sinal que inicia o segundo Mega.
      SerialMega.println("1");

      Serial.println("SINAL 1 ENVIADO PARA MEGA 2!");
    }
  }

  delay(10);
}
