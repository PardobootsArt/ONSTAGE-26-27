#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

// Comunicação com Sullivan
const int RX_MEGA = 19;
const int TX_MEGA = 18;

bool conectadoComC = false;
bool sinal2Recebido = false;
bool sinalFinalRecebido = false;

void conectarNaESPC() {

  Serial.println();
  Serial.println("Tentando conectar na ESP C...");
  Serial.println("Procurando: ESP_C");

  if (SerialBT.connect("ESP_C")) {

    conectadoComC = true;

    Serial.println("================================");
    Serial.println("ESP C CONECTADA!");
    Serial.println("Bluetooth funcionando.");
    Serial.println("Aguardando os robos...");
    Serial.println("================================");

  } else {

    conectadoComC = false;

    Serial.println("NAO CONECTOU NA ESP C.");
    Serial.println("Tentando novamente...");
  }
}

void setup() {

  Serial.begin(115200);

  // ESP B <-> Sullivan
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // ESP B = MASTER
  SerialBT.begin("ESP_B", true);

  Serial.println("================================");
  Serial.println("ESP B INICIADA");
  Serial.println("ESP B = MASTER");
  Serial.println("================================");

  conectarNaESPC();
}

void loop() {

  // ==========================================
  // MANTER BLUETOOTH COM ESP C
  // ==========================================

  if (!SerialBT.connected()) {

    if (conectadoComC) {

      conectadoComC = false;

      Serial.println("ESP C DESCONECTADA!");
      Serial.println("Tentando reconectar...");
    }

    conectarNaESPC();

    delay(2000);
  }

  // ==========================================
  // SULLIVAN -> ESP B
  // ==========================================

  if (SerialMega.available()) {

    String mensagem = SerialMega.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido do Sullivan: ");
    Serial.println(mensagem);

    if (mensagem == "2" && !sinal2Recebido) {

      sinal2Recebido = true;

      Serial.println("2 OBJETOS ENCONTRADOS!");

      if (SerialBT.connected()) {

        SerialBT.println("2");

        Serial.println("SINAL 2 ENVIADO PARA ESP C!");

      } else {

        Serial.println("ERRO: ESP C NAO ESTA CONECTADA!");
      }
    }
  }

  // ==========================================
  // ESP C -> ESP B
  // ==========================================

  if (SerialBT.available()) {

    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP C: ");
    Serial.println(mensagem);

    if (mensagem == "1" && !sinalFinalRecebido) {

      sinalFinalRecebido = true;

      Serial.println("SINAL 1 RECEBIDO DA ESP C!");

      // ESP B -> Sullivan
      SerialMega.println("1");

      Serial.println("SINAL 1 ENVIADO PARA O SULLIVAN!");
    }
  }
}