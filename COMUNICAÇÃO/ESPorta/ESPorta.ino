#include <WiFi.h>
#include <esp_now.h>

typedef struct {
  int comando;
} Mensagem;

Mensagem msg;

uint8_t robo1[] = { 0x3c, 0x71, 0xbf, 0xd0, 0x62, 0x74 };

const int led = 2;

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {

  if (status == ESP_NOW_SEND_SUCCESS){
    Serial.println("Enviado com sucesso");
  }else{
    Serial.println("Falha no envio");
  }
}

void setup() {

  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  pinMode(led, OUTPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peer = {};

  memcpy(peer.peer_addr, robo1, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Erro ao adicionar peer");
  }
}

void loop() {

  if (Serial2.available()) {

    int dado = Serial2.parseInt();

    Serial.print("Recebi: ");
    Serial.println(dado);

    if (dado == 1) {

      digitalWrite(led, HIGH);

      msg.comando = 3;

      esp_now_send(robo1, (uint8_t *)&msg, sizeof(msg));
      delay (500);
      digitalWrite(led, LOW);
      delay (500);
    }
  }
}