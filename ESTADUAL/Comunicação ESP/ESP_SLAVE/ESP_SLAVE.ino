#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

const int RX_MEGA = 16;
const int TX_MEGA = 17;

// MAC da ESP C
uint8_t macESP_C[] = {0x3C, 0x71, 0xBF, 0xD0, 0x62, 0x74};

bool faseComA = true;
bool faseComC = false;
bool sinal1Recebido = false;

void conectarNaESPC() {
  Serial.println("Desconectando da ESP A...");
  SerialBT.disconnect();
  delay(1500);

  SerialBT.end();
  delay(1000);

  Serial.println("ESP B mudando para modo master...");
  SerialBT.begin("ESP_B", true);
  delay(500);

  Serial.println("Conectando na ESP C...");
  if (SerialBT.connect(macESP_C)) {
    faseComA = false;
    faseComC = true;
    Serial.println("ESP C conectada!");
  } else {
    Serial.println("Falha ao conectar na ESP C.");
  }
}

void setup() {
  Serial.begin(115200);
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // ESP B começa como servidor e espera a ESP A.
  SerialBT.begin("ESP_B");

  Serial.println("ESP B iniciada.");
  Serial.println("Aguardando sinal da ESP A...");
}

void loop() {
  // =================================================
  // FASE 1: ESP A -> ESP B -> MEGA 1
  // =================================================
  if (faseComA && SerialBT.available()) {
    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    if (mensagem == "1" && !sinal1Recebido) {
      sinal1Recebido = true;

      Serial.println("SINAL 1 RECEBIDO DA ESP A!");
      SerialMega.println("1");
      Serial.println("SINAL 1 ENVIADO PARA MEGA 1!");

      // Agora a proxima etapa sera Mega 1 -> ESP B -> ESP C.
      conectarNaESPC();
    }
  }

  // =================================================
  // FASE 2: MEGA 1 -> ESP B -> ESP C
  // =================================================
  if (faseComC && SerialMega.available()) {
    String mensagemMega = SerialMega.readStringUntil('\n');
    mensagemMega.trim();

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
