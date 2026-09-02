#include <esp_now.h>
#include <WiFi.h>

HardwareSerial MegaSerial(2);
typedef struct {
  int comando;
} Mensagem;
const int led = 2;

Mensagem msg;

void recebeu(const esp_now_recv_info_t *info,
             const uint8_t *dados,
             int len) {
  Serial.print("Bytes recebidos: ");
  Serial.println(len);

  memcpy(&msg, dados, sizeof(msg));

  Serial.print("Comando recebido: ");
  Serial.println(msg.comando);

  MegaSerial.println(msg.comando);
}

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);

  MegaSerial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.mode(WIFI_STA);

  esp_now_init();

  esp_now_register_recv_cb(recebeu);

}

void loop() {}