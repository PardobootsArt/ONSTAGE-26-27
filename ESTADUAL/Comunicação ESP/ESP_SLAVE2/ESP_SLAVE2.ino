#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

// Comunicação com Randall
const int RX_MEGA = 19;
const int TX_MEGA = 18;

bool conectadoComB = false;
bool sinal2Recebido = false;
bool sinalFinalEnviado = false;

void setup() {

  Serial.begin(115200);

  // ESP C <-> Randall
  SerialMega.begin(115200, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // ESP C = SLAVE
  SerialBT.begin("ESP_C");

  Serial.println("================================");
  Serial.println("ESP C INICIADA");
  Serial.println("ESP C = SLAVE");
  Serial.println("Nome Bluetooth: ESP_C");
  Serial.println("Aguardando ESP B...");
  Serial.println("================================");
}

void loop() {

  // ==========================================
  // VERIFICAR BLUETOOTH
  // ==========================================

  bool estadoAtual = SerialBT.connected();

  if (estadoAtual && !conectadoComB) {

    conectadoComB = true;

    Serial.println("================================");
    Serial.println("ESP B CONECTADA!");
    Serial.println("Bluetooth funcionando.");
    Serial.println("Aguardando os robos...");
    Serial.println("================================");
  }

  if (!estadoAtual && conectadoComB) {

    conectadoComB = false;

    Serial.println("ESP B DESCONECTADA!");
    Serial.println("Aguardando reconexao...");
  }

  // ==========================================
  // ESP B -> ESP C
  // ==========================================

  if (SerialBT.available()) {

    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP B: ");
    Serial.println(mensagem);

    if (mensagem == "2" && !sinal2Recebido) {

      sinal2Recebido = true;

      Serial.println("SINAL 2 RECEBIDO!");

      // ESP C -> Randall
      SerialMega.println("1");

      Serial.println("SINAL 1 ENVIADO PARA O RANDALL!");
    }
  }

  // ==========================================
  // RANDALL -> ESP C
  // ==========================================

  if (sinal2Recebido && SerialMega.available()) {

    String mensagem = SerialMega.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido do Randall: ");
    Serial.println(mensagem);
  }
}