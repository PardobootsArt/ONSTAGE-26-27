#include <SoftwareSerial.h>
SoftwareSerial espSerial (5,6);


// Pinos do sensor ultrassônico
const int trigPin = 10;
const int echoPin = 9;

// Pinos do driver do motor
const int IN1 = 12;
const int IN2 = 13;
const int ENB = 11;

// Pino do LED
const int led = 3;

// Variáveis para armazenar o tempo do eco e a distância
float duration;
float distance;

// Variável que indica se o objeto já foi detectado
// Começa como false (ainda não detectou)
bool objetoDetectado = false;

void setup() {
  espSerial.begin (115200);

  // Inicia a comunicação com o Monitor Serial
  Serial.begin(9600);

  // Define os pinos do ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Define os pinos do motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Define o pino do LED
  pinMode(led, OUTPUT);

  // Garante que motor e LED iniciem desligados
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENB, 0);
  digitalWrite(led, LOW);
}

void loop() {

  // Envia um pulso de 10 microssegundos para o sensor ultrassônico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  // Lê o tempo que o eco demorou para voltar
  // Tempo máximo de espera: 30 ms
  duration = pulseIn(echoPin, HIGH, 30000);

  // Se não houver leitura, sai desta repetição do loop
  if (duration == 0) {
    return;
  }

  // Calcula a distância em centímetros
  distance = duration * 0.034 / 2;

  // Mostra a distância no Monitor Serial
  Serial.print("Distancia: ");
  Serial.println(distance);

  // Executa somente se:
  // 1. O objeto estiver a menos de 10 cm
  // 2. Ainda não tiver sido detectado anteriormente
  if (distance < 10 && objetoDetectado == false) {
    espSerial.println (1);
    // Marca que o objeto já foi detectado
    // Assim o motor não será acionado novamente
    objetoDetectado = true;

    // Liga o LED e mantém aceso
    digitalWrite(led, HIGH);

    // Define o sentido de rotação do motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    // Liga o motor com velocidade 120 (0 a 255)
    analogWrite(ENB, 120);

    // Mantém o motor ligado por 2 segundos
    delay(1600);

    // Desliga o motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENB, 0);

    // Mensagem para confirmar a execução
    Serial.println("Objeto detectado. Motor acionado uma unica vez.");
  }

  // Pequena pausa antes da próxima leitura
  delay(100);
}