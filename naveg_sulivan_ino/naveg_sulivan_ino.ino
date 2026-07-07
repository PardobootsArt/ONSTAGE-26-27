//giroscopio
#include <Wire.h>

const int MPU = 0x68;

float anguloZ = 0.0;
unsigned long ultimoTempo;

float offsetGz = 0;

//radar
#include <Servo.h>

Servo meuServo;

#define TRIG 3
#define ECHO 2
int posicaoAtual = 25;
bool sentidoHorario = true;

//correcao
float Kp = 5.0;
float Kd = 0.5;
int cronometro;

//FUNÇÕES DA PROGRAMAÇÃO
int lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH);

  return duracao * 0.034 / 2;
}

bool varreduraCompleta = false;
int procurarObjeto() {

  while (true) {

    meuServo.write(posicaoAtual);
    delay(15);

    int distancia = lerDistancia();

    if (distancia > 0 && distancia < 20) {

      delay(1000);  // pausa olhando para o objeto
      varreduraCompleta = false;
      return posicaoAtual;
    }

    if (sentidoHorario) {

      posicaoAtual += 4;

      if (posicaoAtual >= 155) {
        posicaoAtual = 155;
        sentidoHorario = false;
      }

    } else {

      posicaoAtual -= 4;

      if (posicaoAtual <= 25) {
        posicaoAtual = 25;
        sentidoHorario = true;
        varreduraCompleta = true;
      }
    }
  }
  return -1;
}


//FUNÇÃO PARA O GIROSCÓPIO
void iniciarMPU() {
  Wire.begin();

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Calibração do giroscópio
  long soma = 0;

  for (int i = 0; i < 1000; i++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x47);  // GyZ
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 2, true);

    int16_t Gz = Wire.read() << 8 | Wire.read();

    soma += Gz;
    delay(2);
  }

  offsetGz = soma / 1000.0;

  anguloZ = 0;
  ultimoTempo = micros();
}

float obterAngulo() {

  Wire.beginTransmission(MPU);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 2, true);

  int16_t Gz = Wire.read() << 8 | Wire.read();

  float velocidadeAngular = (Gz - offsetGz) / 131.0;

  unsigned long agora = micros();
  float dt = (agora - ultimoTempo) / 1000000.0;
  ultimoTempo = agora;

  anguloZ += velocidadeAngular * dt;

  while (anguloZ >= 360)
    anguloZ -= 360;

  while (anguloZ < 0)
    anguloZ += 360;


  // Converte para -180 a +180
  float anguloCorrigido = anguloZ;

  if (anguloCorrigido > 180)
    anguloCorrigido -= 360;

  return anguloCorrigido;
}

// MOTORES
int motorDF1 = 26;
int motorDF2 = 28;
int motorEF1 = 22;
int motorEF2 = 24;
int motorDT1 = 34;
int motorDT2 = 36;
int motorET1 = 30;
int motorET2 = 32;

int enbEF = 13;
int enbDF = 12;
int enbET = 11;
int enbDT = 10;

int servo = 38;

void setup() {
  //CRONOMETRO
  cronometro = 0;

  //COMUNICAÇÃO
  Serial.begin(9600);
  Serial2.begin(9600);


  iniciarMPU();

  Serial.println("MPU pronto!");

  //RADAR
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  meuServo.attach(servo);
  meuServo.write(90);
  delay(300);

  //MOTORES
  pinMode(motorEF1, OUTPUT);
  pinMode(motorEF2, OUTPUT);
  pinMode(motorDF1, OUTPUT);
  pinMode(motorDF2, OUTPUT);
  pinMode(motorET1, OUTPUT);
  pinMode(motorET2, OUTPUT);
  pinMode(motorDT1, OUTPUT);
  pinMode(motorDT2, OUTPUT);

  pinMode(enbEF, OUTPUT);
  pinMode(enbDF, OUTPUT);
  pinMode(enbET, OUTPUT);
  pinMode(enbDT, OUTPUT);

  pinMode(20, INPUT);
  pinMode(21, INPUT);
}

void temporizador(int Z, int C) {
  if (C == 1) {
    cronometro++;
  } else if (Z == 1) {
    cronometro = 0;
  }
}

void esquerda(int velo, int tempo) {
  temporizador(1, 0);
  float ultimoError = 0;
  while (cronometro < tempo) {
    temporizador(0, 1);
    float error = obterAngulo();
    Serial.print(" Error: ");
    Serial.print(obterAngulo());
    Serial.print("Tempo: ");
    Serial.print(cronometro);
    float correcaoP = Kp * error;
    float correcaoD = Kd * (error - ultimoError);
    float correcaoFinal = correcaoP + correcaoD;
    float velDir = velo - correcaoFinal;
    float velEsq = velo + correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print(" Direita: ");
    Serial.print(velDir);
    Serial.print(" Esquerda: ");
    Serial.println(velEsq);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velDir);
    analogWrite(enbET, velEsq);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
    ultimoError = error;
  }
  digitalWrite(motorEF1, LOW);
  digitalWrite(motorEF2, LOW);
  digitalWrite(motorDF1, LOW);
  digitalWrite(motorDF2, LOW);
  digitalWrite(motorET1, LOW);
  digitalWrite(motorET2, LOW);
  digitalWrite(motorDT1, LOW);
  digitalWrite(motorDT2, LOW);
}

void direita(int velo, int tempo) {
  temporizador(1, 0);
  float ultimoError = 0;
  while (cronometro < tempo) {
    temporizador(0, 1);
    float error = obterAngulo();
    Serial.print(" Error: ");
    Serial.print(obterAngulo());
    Serial.print("Tempo: ");
    Serial.print(cronometro);
    float correcaoP = Kp * error;
    float correcaoD = Kd * (error - ultimoError);
    float correcaoFinal = correcaoP + correcaoD;
    float velDir = velo + correcaoFinal;
    float velEsq = velo - correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print(" Direita: ");
    Serial.print(velDir);
    Serial.print(" Esquerda: ");
    Serial.println(velEsq);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velDir);
    analogWrite(enbET, velEsq);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
    ultimoError = error;
  }

  digitalWrite(motorEF1, LOW);
  digitalWrite(motorEF2, LOW);
  digitalWrite(motorDF1, LOW);
  digitalWrite(motorDF2, LOW);
  digitalWrite(motorET1, LOW);
  digitalWrite(motorET2, LOW);
  digitalWrite(motorDT1, LOW);
  digitalWrite(motorDT2, LOW);
}

void frente(int velo, int tempo) {
  temporizador(1, 0);
  float ultimoError = 0;
  while (cronometro < tempo) {
    float error = obterAngulo();
    Serial.print(" Angulo: ");
    Serial.print(error);
    temporizador(0, 1);
    Serial.print(" Tempo: ");
    Serial.print(cronometro);
    float correcaoP = Kp * error;
    float correcaoD = Kd * (error - ultimoError);
    float correcaoFinal = correcaoP + correcaoD;
    float velDir = velo + correcaoFinal;
    float velEsq = velo - correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print(" Direita: ");
    Serial.print(velDir);
    Serial.print(" Esquerda: ");
    Serial.println(velEsq);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velEsq);
    analogWrite(enbET, velDir);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
    ultimoError = error;
  }

  digitalWrite(motorEF1, LOW);
  digitalWrite(motorEF2, LOW);
  digitalWrite(motorDF1, LOW);
  digitalWrite(motorDF2, LOW);
  digitalWrite(motorET1, LOW);
  digitalWrite(motorET2, LOW);
  digitalWrite(motorDT1, LOW);
  digitalWrite(motorDT2, LOW);
}


void tras(int velo, int tempo) {
  temporizador(1, 0);
  float ultimoError = 0;
  while (cronometro < tempo) {
    float error = obterAngulo();
    Serial.print(" Angulo: ");
    Serial.print(error);
    temporizador(0, 1);
    Serial.print(" Tempo: ");
    Serial.print(cronometro);
    float correcaoP = Kp * error;
    float correcaoD = Kd * (error - ultimoError);
    float correcaoFinal = correcaoP + correcaoD;
    float velDir = velo - correcaoFinal;
    float velEsq = velo + correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print(" Direita: ");
    Serial.print(velDir);
    Serial.print(" Esquerda: ");
    Serial.println(velEsq);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velEsq);
    analogWrite(enbET, velDir);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorDT2, HIGH);
    digitalWrite(motorDT1, LOW);
    ultimoError = error;
  }

  digitalWrite(motorEF1, LOW);
  digitalWrite(motorEF2, LOW);
  digitalWrite(motorDF1, LOW);
  digitalWrite(motorDF2, LOW);
  digitalWrite(motorET1, LOW);
  digitalWrite(motorET2, LOW);
  digitalWrite(motorDT1, LOW);
  digitalWrite(motorDT2, LOW);
}

int calcularTempo(int angulo) {

  int erro = abs(angulo - 90);
  float fator = cos(radians(erro));
  int tempo = 15 * cos(radians(abs(angulo - 90)));
  tempo = constrain(tempo, 5, 20);
  return tempo;
}

bool frenteLivre() {

  meuServo.write(90);  // Centraliza o sensor
  delay(80);           // Tempo suficiente para o servo chegar

  int distancia = lerDistancia();

  if (distancia > 20 || distancia == 0) {
    return true;  // Frente livre
  }

  return false;  // Ainda há obstáculo
}

void loop() {

  if (Serial2.available()) {
    int value = Serial2.parseInt();
    Serial.print("recebi: ");
    Serial.println(value);
  }
}