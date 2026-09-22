#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

// Comunicação ESP B <-> Sullivan
const int RX_MEGA = 16;
const int TX_MEGA = 17;

bool conectadoComA = false;
bool sinalRecebido = false;

void setup() {
  Serial.begin(115200);

  // ESP B <-> Sullivan
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // ESP B = SLAVE
  SerialBT.begin("ESP_B");

  Serial.println("================================");
  Serial.println("ESP B INICIADA");
  Serial.println("ESP B = SLAVE");
  Serial.println("Aguardando ESP A...");
  Serial.println("================================");
}

void loop() {

  // ==========================================
  // VERIFICAR CONEXAO COM ESP A
  // ==========================================

  bool estadoAtual = SerialBT.connected();

  if (estadoAtual && !conectadoComA) {

    conectadoComA = true;

    Serial.println("================================");
    Serial.println("ESP A CONECTADA!");
    Serial.println("Bluetooth funcionando.");
    Serial.println("Aguardando sinal da porta...");
    Serial.println("================================");
  }

  if (!estadoAtual && conectadoComA) {

    conectadoComA = false;

    Serial.println("ESP A DESCONECTADA!");
    Serial.println("Aguardando reconexao...");
  }

  // ==========================================
  // ESP A -> ESP B -> SULLIVAN
  // ==========================================

  if (SerialBT.available()) {

    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP A: ");
    Serial.println(mensagem);

    if (mensagem == "1" && !sinalRecebido) {
      
      sinalRecebido = true;

      Serial.println("SINAL 1 RECEBIDO DA PORTA!");

      // Envia para o Arduino do Sullivan
      SerialMega.println("1");

      Serial.println("SINAL 1 ENVIADO PARA O SULLIVAN!");
    }
  }
}