#include <esp_now.h>
#include <WiFi.h>

HardwareSerial MegaSerial(2);
typedef struct {
  int comando;
} Mensagem;
const int led = 2;

Mensagem msgRecebida;
Mensagem msgEnviar;

uint8_t robo2[] = { 0x3c, 0x71, 0xbf, 0x45, 0x96, 0x94 };

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {

  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Enviado com sucesso");
  } else {
    Serial.println("Falha no envio");
  }
}


void recebeu(const esp_now_recv_info_t *info,
             const uint8_t *dados,
             int len) {
  Serial.print("Bytes recebidos: ");
  Serial.println(len);

  if (len != sizeof(Mensagem)) {
    Serial.println("Tamanho incorreto");
    return;
  }
  memcpy(&msgRecebida, dados, sizeof(msgRecebida));

  Serial.print("Comando recebido: ");
  Serial.println(msgRecebida.comando);

  MegaSerial.println(msgRecebida.comando);
}

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  MegaSerial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ESP-NOW");
    return;
  }

  //=======RECEBER==========
  esp_now_register_recv_cb(recebeu);
  //=======MANDAR===========
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peer = {};


  memcpy(peer.peer_addr, robo2, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Erro ao adicionar peer");
  }
}

void loop() {
  if (MegaSerial.available()) {

    int dado = MegaSerial.parseInt();

    Serial.print("Recebi: ");
    Serial.println(dado);

    if (dado == 2) {

      digitalWrite(led, HIGH);

      msgEnviar.comando = dado;

      esp_now_send(robo2, (uint8_t *)&msgEnviar, sizeof(msgEnviar));
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
  }
}