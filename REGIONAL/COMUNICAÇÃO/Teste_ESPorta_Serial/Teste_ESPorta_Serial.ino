// this sample code provided by www.programmingboss.com
#define RXp2 16
#define TXp2 17
const int led = 2;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  pinMode (led, OUTPUT);
}
void loop() {
  if (Serial2.available() > 0) {
    digitalWrite (led, HIGH);
    Serial.println("Message Received: ");
    Serial.println(Serial2.readString());
    delay (1000);
    digitalWrite (led, LOW);
    delay (1000);
  }
}