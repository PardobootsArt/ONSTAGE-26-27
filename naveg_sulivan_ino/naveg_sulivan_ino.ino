//giroscopio
#include <Wire.h>
#include <SoftwareSerial.h>

const int MPU = 0x68;

float anguloZ = 0.0;
unsigned long ultimoTempo;

float offsetGz = 0;

//=============RADAR==============
#include <Servo.h>

Servo meuServo;

#define TRIG 3
#define ECHO 2
int posicaoAtual = 25;
bool sentidoHorario = true;

//===========CORREÇÃO===========
float Kp = 1.2;
float Kd = 0.05;
int cronometro;
float anguloReferencia = 0;
float ultimoErro = 0;
unsigned long ultimoControle = 0;

//==============FUNÇÕES DA PROGRAMAÇÃO=============
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

      delay(150);
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
//===================SETUP========================
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

float correcao() {
  float ang = obterAngulo();
  float erro = anguloReferencia - ang;

  unsigned long agora = millis();
  float dt = (agora - ultimoControle) / 1000.0;
  if (dt <= 0) dt = 0.001;

  float deriv = (erro - ultimoErro) / dt;
  ultimoErro = erro;
  ultimoControle = agora;

  return Kp * erro + Kd * deriv;
}

void pararMotores() {
  analogWrite(enbEF, 0);
  analogWrite(enbDF, 0);
  analogWrite(enbET, 0);
  analogWrite(enbDT, 0);
  digitalWrite(motorEF1, LOW);
  digitalWrite(motorEF2, LOW);
  digitalWrite(motorDF1, LOW);
  digitalWrite(motorDF2, LOW);
  digitalWrite(motorET1, LOW);
  digitalWrite(motorET2, LOW);
  digitalWrite(motorDT1, LOW);
  digitalWrite(motorDT2, LOW);
}

void esquerda(int velo, int tempoMs) {
  anguloReferencia = obterAngulo();
  ultimoErro = 0;
  ultimoControle = millis();
  unsigned long inicio = millis();
  while (millis() - inicio < tempoMs) {
    float c = correcao();
    ;
    float velDir = velo + c;
    float velEsq = velo - c;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velDir);
    analogWrite(enbET, velEsq);
    analogWrite(enbDT, velDir);
  }
  pararMotores();
}

void direita(int velo, int tempoMs) {
  anguloReferencia = obterAngulo();
  ultimoErro = 0;
  ultimoControle = millis();
  unsigned long inicio = millis();

  while (millis() - inicio < tempoMs) {
    float c = correcao();
    float velDir = velo - c;
    float velEsq = velo + c;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velDir);
    analogWrite(enbET, velEsq);
    analogWrite(enbDT, velDir);
  }
  pararMotores();
}

void frente(int velo, int tempoMs) {
  anguloReferencia = obterAngulo();
  ultimoErro = 0;
  ultimoControle = millis();
  unsigned long inicio = millis();

  while (millis() - inicio < tempoMs) {
    float c = correcao();
    float velDir = velo - c;
    float velEsq = velo + c;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velEsq);
    analogWrite(enbET, velDir);
    analogWrite(enbDT, velDir);
  }
  pararMotores();
}


void tras(int velo, int tempoMs) {
  anguloReferencia = obterAngulo();
  ultimoErro = 0;
  ultimoControle = millis();
  unsigned long inicio = millis();

  while (millis() - inicio < tempoMs) {
    float c = correcao();
    float velDir = velo + c;
    float velEsq = velo - c;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorDT2, HIGH);
    digitalWrite(motorDT1, LOW);
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velEsq);
    analogWrite(enbET, velDir);
    analogWrite(enbDT, velDir);
  }

  pararMotores();
}

int calcularTempo(int angulo) {

  int erro = abs(angulo - 90);
  float fator = cos(radians(erro));
  int tempo = 15000 * cos(radians(abs(angulo - 90)));
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
  int contagemObjeto = 0;
  while (contagemObjeto < 3) {
    int grauss = procurarObjeto();
    int time = calcularTempo(grauss);
    if (grauss != -1) {
      if (grauss < 90) {
        //objeto na esquerda, ir para direita
        direita(70, time);
        delay(200);
        if (frenteLivre() == true){
          frente (80, 500);
          contagemObjeto ++;
        } else {
          direita (70, 200);
          delay (200);
          frente (80, 500);
        }
      }

      if (grauss > 90) {
        //objeto na direita, ir para esquerda
        esquerda(70, time);
        delay(200);
        if (frenteLivre() == true){
          frente (80, 500);
          contagemObjeto ++;
        } else {
          esquerda (70, 200);
          delay (200);
          frente (80, 500);
        }
      }
    } else if (varreduraCompleta) {
      frente(80, 200);
      varreduraCompleta = false;
    }
  }
  delay(2000);
}