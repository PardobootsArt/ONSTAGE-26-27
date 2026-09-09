#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// ================= SERIAL COM MEGA 1 =================
// ESP B TX GPIO17 -> Mega 1 RX1 (19)
// ESP B RX GPIO16 <- Mega 1 TX1 (18)
HardwareSerial SerialMega(2);

const int RX_MEGA = 16;
const int TX_MEGA = 17;

// ================= BLUETOOTH =================
uint8_t macESP_A[] = {0x3C, 0x71, 0xBF, 0x45, 0x96, 0x94};
const char *nomeESP_C = "ESP_C";

bool faseComA = true;
bool faseComC = false;
bool sinal1Recebido = false;

void conectarNaESP_C() {
  Serial.println("Desconectando da ESP A...");

  if (SerialBT.disconnect()) {
    Serial.println("ESP A desconectada.");
  } else {
    Serial.println("ESP A ja estava desconectada.");
  }

  delay(1500);

  // Encerra o Bluetooth que estava funcionando como slave.
  SerialBT.end();
  delay(1000);

  // Agora a ESP B vira MASTER para conectar na ESP C.
  SerialBT.begin("ESP_B", true);
  delay(500);

  Serial.println("ESP B agora esta em modo MASTER.");
  Serial.print("Tentando conectar na ESP C pelo nome: ");
  Serial.println(nomeESP_C);

  if (SerialBT.connect(nomeESP_C)) {
    Serial.println("ESP B conectada na ESP C!");
    faseComA = false;
    faseComC = true;
  } else {
    Serial.println("Nao conectou na ESP C. Tentando novamente...");
  }
}

void setup() {
  Serial.begin(115200);

  // Serial com Mega 1
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // Primeiro a ESP B funciona como SLAVE para a ESP A.
  SerialBT.begin("ESP_B");

  Serial.println("ESP B iniciada.");
  Serial.println("Aguardando ESP A...");
}

void loop() {

  // =====================================================
  // FASE 1: ESP A -> ESP B -> MEGA 1
  // =====================================================
  if (faseComA) {

    if (SerialBT.available()) {
      String mensagem = SerialBT.readStringUntil('\n');
      mensagem.trim();

      Serial.print("Recebido da ESP A: ");
      Serial.println(mensagem);

      if (mensagem == "1" && !sinal1Recebido) {
        sinal1Recebido = true;

        Serial.println("SINAL 1 RECEBIDO DA ESP A!");

        // Envia o sinal para o Mega 1 iniciar.
        SerialMega.println("1");
        Serial.println("SINAL 1 ENVIADO PARA MEGA 1!");

        // Depois de receber o 1, troca A pela C.
        conectarNaESP_C();
      }
    }
  }

  // =====================================================
  // FASE 2: MEGA 1 -> ESP B -> ESP C
  // =====================================================
  if (faseComC) {

    if (SerialMega.available()) {
      String mensagemMega = SerialMega.readStringUntil('\n');
      mensagemMega.trim();

      Serial.print("Recebido do Mega 1: ");
      Serial.println(mensagemMega);

      if (mensagemMega == "2") {
        Serial.println("SINAL 2 RECEBIDO DO MEGA 1!");

        if (SerialBT.connected()) {
          SerialBT.println("2");
          Serial.println("SINAL 2 ENVIADO PARA ESP C!");
        } else {
          Serial.println("ESP C nao esta conectada!");
        }
      }
    }
  }

  delay(10);
}
