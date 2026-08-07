#define bracoE1 49
#define bracoE2 47
#define bracoD1 51
#define bracoD2 53
#define enbracoD 4
#define enbracoE 5
#define tronco1 45
#define tronco2 43
#define enbtronco 6



void setup() {
  pinMode(bracoE1, OUTPUT);
  pinMode(bracoE2, OUTPUT);
  pinMode(bracoD1, OUTPUT);
  pinMode(bracoD2, OUTPUT);
  pinMode(enbracoD, OUTPUT);
  pinMode(enbracoE, OUTPUT);
  pinMode(tronco1, OUTPUT);
  pinMode(tronco2, OUTPUT);
  pinMode(enbtronco, OUTPUT);
  analogWrite(enbracoD, 95);
  analogWrite(enbracoE, 95);
  analogWrite(enbtronco, 175);
}

void loop() {
  digitalWrite(bracoE1, HIGH);
  digitalWrite(bracoE2, LOW);
  digitalWrite(bracoD1, HIGH);
  digitalWrite(bracoD2, LOW);
  digitalWrite(tronco1, HIGH);
  digitalWrite(tronco2, LOW);
  delay(550);
  digitalWrite(bracoE1, LOW);
  digitalWrite(bracoE2, LOW);
  digitalWrite(bracoD1, LOW);
  digitalWrite(bracoD2, LOW);
  digitalWrite(tronco1, LOW);
  digitalWrite(tronco2, LOW);
  delay(500);
  digitalWrite(bracoE1, LOW);
  digitalWrite(bracoE2, HIGH);
  digitalWrite(bracoD1, LOW);
  digitalWrite(bracoD2, HIGH);
  digitalWrite(tronco1, LOW);
  digitalWrite(tronco2, HIGH);
  delay(550);
  digitalWrite(bracoE1, LOW);
  digitalWrite(bracoE2, LOW);
  digitalWrite(bracoD1, LOW);
  digitalWrite(bracoD2, LOW);
  digitalWrite(tronco1, LOW);
  digitalWrite(tronco2, LOW);
  delay(500);
}
