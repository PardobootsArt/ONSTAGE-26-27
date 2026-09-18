// Ultrassonico
const int trigPin = 18;
const int echoPin = 19;

// Motor da porta
const int IN1 = 2;
const int IN2 = 15;
const int ENA = 4;

// Fita LED
const int led = 33;

bool objetoDetectado = false;
bool portaAberta = false;

long lerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  if (duracao == 0) return -1;
  return duracao * 0.034 / 2;
}

void abrirPorta() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
  delay(1000);
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  portaAberta = true;
  Serial.println("PORTA ABERTA!");
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(led, OUTPUT);

  digitalWrite(led, LOW);

  Serial.println("ESP A iniciada.");
}

void loop() {
  long distancia = lerDistancia();

  if (distancia > 0) {
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  if (distancia > 0 && distancia < 10 && !objetoDetectado) {
    objetoDetectado = true;
    Serial.println("OBJETO DETECTADO!");
    digitalWrite(led, HIGH);
    abrirPorta();
    delay(1000);
  }

  if (distancia > 15) {
    objetoDetectado = false;
  }

  delay(100);
}