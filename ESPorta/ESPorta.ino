#include <esp_now.h>
#include <WiFi.h>

HardwareSerial MegaSerial(2);

typedef struct{
  int comando;
}Mensagem;

Mensagem msg;

// MAC do Robô 1
uint8_t robo1[] = {0x3c,0x71,0xbf,0xd0,0x62,0x74};

// MAC do Robô 2
uint8_t robo2[] = {0x3c,0x71,0xbf,0x45,0x96,0x94};

void setup(){

  MegaSerial.begin(115200,SERIAL_8N1,16,17);

  WiFi.mode(WIFI_STA);

  esp_now_init();

  esp_now_peer_info_t peer;

  memcpy(peer.peer_addr, robo1,6);
  peer.channel=0;
  peer.encrypt=false;
  esp_now_add_peer(&peer);

  memcpy(peer.peer_addr, robo2,6);
  esp_now_add_peer(&peer);

}

void loop(){

  if(MegaSerial.available()){

    msg.comando = MegaSerial.parseInt();

    esp_now_send(robo1,(uint8_t*)&msg,sizeof(msg));

    esp_now_send(robo2,(uint8_t*)&msg,sizeof(msg));

  }

}