const int trigPin = 10;
const int echoPin = 9;
int Leds[] = {4, 6, 8, 12};
float duration, distance;

// L298N
#define ENA 2
#define IN1 11
#define IN2 3

void setup() {

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(Leds[i], OUTPUT);
  }

  Serial.begin(9600);
}

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  distance = duration * 0.034 / 2;

  Serial.println(distance);

  if (distance > 0 && distance < 10) {

    // Liga motor primeiro
    analogWrite(ENA, 255);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    delay(200);

    // Depois liga LEDs
    for (int i = 0; i < 4; i++) {
      digitalWrite(Leds[i], HIGH);
      delay(50);
    }

  } else {

    for (int i = 0; i < 4; i++) {
      digitalWrite(Leds[i], LOW);
    }

    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  delay(100);
}