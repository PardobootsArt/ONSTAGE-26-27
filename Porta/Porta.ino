const int trigPin = 10;
const int echoPin = 9;
const int IN1 = 12;
const int IN2 = 13;
const int ENB = 11;
const int led = 3;
float duration, distance;

void setup() {

  Serial.begin(9600);

  // Ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  // LED
  pinMode(led, OUTPUT);
}

void loop() {

  // Disparo do sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Leitura do eco
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.println(distance);
  delay(1000);
  // Se detectar a cápsula
  if (distance < 10) {
     // liga LED
    digitalWrite(IN2, HIGH); // abre porta
    digitalWrite(IN1, LOW);
    analogWrite(ENB, 120);
    analogWrite(led,255);
    delay(2000);
    digitalWrite(IN2, LOW); 
    digitalWrite(IN1, LOW);

  } else {

     // desliga LED e motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(led, 0);
  }

  delay(100);
}
