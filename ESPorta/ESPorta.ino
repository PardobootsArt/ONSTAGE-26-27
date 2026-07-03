#include <esp_now.h>
#include <WiFi.h>

typedef struct{
  int comando;
}Mensagem;

Mensagem msg;
// MAC do Robô 1
uint8_t robo1[] = { 0x3c, 0x71, 0xbf, 0xd0, 0x62, 0x74 };

const int led = 2;
void setup() {
  pinMode (led, OUTPUT);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.mode(WIFI_STA);

  esp_now_init();

  esp_now_peer_info_t slaveInfo;

  memcpy(slaveInfo.peer_addr, robo1, 6);
  slaveInfo.channel = 0;
  slaveInfo.encrypt = false;
  esp_now_add_peer(&slaveInfo);
}

void loop() {
  if (Serial2.available() > 0) {
    digitalWrite (led, HIGH);
    delay (1500);
    msg.comando = 3;
    esp_now_send(robo1, (uint8_t*)&msg, sizeof(msg));
    digitalWrite (led, LOW);
  }
}