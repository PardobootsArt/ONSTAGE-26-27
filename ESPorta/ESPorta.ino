#include <esp_now.h>
#include <WiFi.h>

<<<<<<< HEAD
typedef struct{
=======
HardwareSerial UnoSerial(2);

typedef struct {
>>>>>>> 144595b170fda105e5ed985ca7366ca09a096358
  int comando;
} Mensagem;

Mensagem msg;
// MAC do Robô 1
uint8_t robo1[] = { 0x3c, 0x71, 0xbf, 0xd0, 0x62, 0x74 };

<<<<<<< HEAD
const int led = 2;
void setup() {
  pinMode (led, OUTPUT);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
=======
// MAC do Robô 2
uint8_t robo2[] = { 0x3c, 0x71, 0xbf, 0x45, 0x96, 0x94 };

void setup() {
  Serial.begin (9600);

  UnoSerial.begin(9600, SERIAL_8N1, 16, 17);
>>>>>>> 144595b170fda105e5ed985ca7366ca09a096358

  WiFi.mode(WIFI_STA);

  esp_now_init();

<<<<<<< HEAD
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
=======
  esp_now_peer_info_t peer;

  memcpy(peer.peer_addr, robo1, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  memcpy(peer.peer_addr, robo2, 6);
  esp_now_add_peer(&peer);
}

void loop() {

  if (UnoSerial.available()) {

    msg.comando = UnoSerial.parseInt();
    Serial.print ("recebi: ");
    Serial.println (msg.comando);

    esp_now_send(robo1, (uint8_t*)&msg, sizeof(msg));

    Serial.print("Resultado envio Robo1: ");
        
>>>>>>> 144595b170fda105e5ed985ca7366ca09a096358
  }
}