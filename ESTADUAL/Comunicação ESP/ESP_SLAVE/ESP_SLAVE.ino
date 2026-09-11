#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
HardwareSerial SerialMega(2);

const int RX_MEGA = 16;
const int TX_MEGA = 17;

uint8_t macESP_C[] = {0x3C, 0x71, 0xBF, 0xD0, 0x62, 0x74};

bool aguardandoSullivan = true;
bool conectadoComC = false;
bool sinal2Recebido = false;
bool sinalFinalRecebido = false;

void conectarNaESPC() {
  Serial.println("Mudando ESP B para MASTER...");

  SerialBT.disconnect();
  delay(1000);
  SerialBT.end();
  delay(500);

  SerialBT.begin("ESP_B", true);
  delay(500);

  Serial.println("Conectando na ESP C pelo MAC...");

  if (SerialBT.connect(macESP_C)) {
    conectadoComC = true;
    aguardandoSullivan = false;

    Serial.println("ESP C CONECTADA!");
    SerialBT.println("2");
    Serial.println("SINAL 2 ENVIADO PARA ESP C!");
  } else {
    Serial.println("FALHA AO CONECTAR NA ESP C!");
  }
}

void setup() {
  Serial.begin(115200);
  SerialMega.begin(9600, SERIAL_8N1, RX_MEGA, TX_MEGA);

  // Primeiro: ESP B como SLAVE, esperando o Sullivan.
  SerialBT.begin("ESP_B");

  Serial.println("ESP B INICIADA.");
  Serial.println("Aguardando sinal 2 do Sullivan...");
}

void loop() {
  // Sullivan -> ESP B
  if (aguardandoSullivan && SerialMega.available()) {
    String mensagem = SerialMega.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido do Sullivan: ");
    Serial.println(mensagem);

    if (mensagem == "2" && !sinal2Recebido) {
      sinal2Recebido = true;
      Serial.println("2 OBJETOS ENCONTRADOS!");
      conectarNaESPC();
    }
  }

  // ESP C -> ESP B
  if (conectadoComC && SerialBT.available()) {
    String mensagem = SerialBT.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP C: ");
    Serial.println(mensagem);

    if (mensagem == "1" && !sinalFinalRecebido) {
      sinalFinalRecebido = true;
      Serial.println("SINAL 1 RECEBIDO DA ESP C!");
      SerialMega.println("1");
      Serial.println("SINAL 1 ENVIADO PARA O SULLIVAN!");
    }
  }
}
