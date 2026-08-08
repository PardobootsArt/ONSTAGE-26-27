//giroscopio
#include <Wire.h>
const int MPU = 0x68;

float anguloZ = 0.0;
unsigned long ultimoTempo;

float offsetGz = 0;
//=============MOVIMENTAÇÃO=============
#define bracoE1 49
#define bracoE2 47
#define bracoD1 51
#define bracoD2 53
#define enbracoD 4
#define enbracoE 5
#define tronco1 45
#define tronco2 43
#define enbtronco 6

#define yTronco 2
#define bTronco 9
volatile long degrees;

//=============RADAR==============
#include <Servo.h>

Servo meuServo;

#define TRIG 5
#define ECHO 4
int posicaoAtual = 25;
bool sentidoHorario = true;
int contagemObjeto;
int repeticao;

//===========CORREÇÃO===========
float Kp = 1.2;
float Kd = 0.05;
float anguloReferencia = 0;
float ultimoErro = 0;
unsigned long ultimoControle = 0;

//==============FUNÇÕES DA PROGRAMAÇÃO==========================
//========================TRONCO===========
void countDegrees() {
  if (digitalRead(yTronco) == digitalRead(bTronco)) {
    degrees++;
  } else {
    degrees--;
  }
}

void girarTronco(int speed, int anguloo) {
  analogWrite(enbtronco, speed);
  if (anguloo > degrees) {
    while (degrees < anguloo) {
      digitalWrite(tronco1, HIGH);
      digitalWrite(tronco2, LOW);
    }
    digitalWrite(tronco1, LOW);
    digitalWrite(tronco2, LOW);
  }
  if (anguloo < degrees) {
    while (degrees > anguloo) {
      digitalWrite(tronco1, LOW);
      digitalWrite(tronco2, HIGH);
    }
    digitalWrite(tronco1, LOW);
    digitalWrite(tronco2, LOW);
  }
}
//====================RADAR===================
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
    int oi = posicaoAtual * (-2);
    girarTronco(110, oi);
    delay(15);

    int distancia = lerDistancia();

    if (distancia > 0 && distancia < 20) {

      delay(150);
      varreduraCompleta = false;
      return posicaoAtual;
    }

    if (sentidoHorario == true) {

      posicaoAtual += 4;

      if (posicaoAtual >= 145) {
        posicaoAtual = 155;
        sentidoHorario = false;
      }

    } else if (sentidoHorario == false) {

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


//==================GIROSCÓPIO=================
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

// ==================MOTORES====================
int motorEF1 = 26;
int motorEF2 = 28;
int motorET1 = 22;
int motorET2 = 24;
int motorDF1 = 34;
int motorDF2 = 36;
int motorDT1 = 30;
int motorDT2 = 32;

int enbET = 13;
int enbEF = 12;
int enbDT = 11;
int enbDF = 10;

int servo = 38;
//===============================================SETUP=========================================
void setup() {
  //COMUNICAÇÃO
  Serial.begin(115200);
  Serial2.begin(115200);

  iniciarMPU();
  Serial.println("MPU pronto!");

  //=========MOVIMENTAÇÃO==========
  pinMode(bracoE1, OUTPUT);
  pinMode(bracoE2, OUTPUT);
  pinMode(bracoD1, OUTPUT);
  pinMode(bracoD2, OUTPUT);

  pinMode(enbracoD, OUTPUT);
  pinMode(enbracoE, OUTPUT);

  pinMode(tronco1, OUTPUT);
  pinMode(tronco2, OUTPUT);
  pinMode(enbtronco, OUTPUT);

  pinMode(yTronco, INPUT);
  pinMode(bTronco, INPUT);
  degrees = 0;
  attachInterrupt(digitalPinToInterrupt(yTronco), countDegrees, CHANGE);
  analogWrite(enbtronco, 100);

  //============RADAR===========
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  meuServo.attach(servo);
  meuServo.write(90);
  delay(300);

  //==============MOTORES=================
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
  contagemObjeto = 0;
  repeticao = 0;
  //frente (80, 1000);
  analogWrite(enbtronco, 175);
  delay(150);
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
//ok
void esquerda(int velo, int tempoMs) {
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
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velEsq);
    analogWrite(enbET, velDir);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
  }
  pararMotores();
}
//ok
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
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velEsq);
    analogWrite(enbET, velDir);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
  }
  pararMotores();
}
//ok
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
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velDir);
    analogWrite(enbET, velEsq);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
  }
  pararMotores();
}
//ok
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
    analogWrite(enbEF, velEsq);
    analogWrite(enbDF, velDir);
    analogWrite(enbET, velEsq);
    analogWrite(enbDT, velDir);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorDT2, HIGH);
    digitalWrite(motorDT1, LOW);
  }

  pararMotores();
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
    int recieve = Serial2.parseInt();
    if (recieve == 3) {
      delay(3000);
      direita(100, 2500);
      delay(500);
      if (repeticao == 0) {
        while (contagemObjeto <= 3) {
          int grauss = procurarObjeto();
          int time = calcularTempo(grauss);
          if (grauss != -1) {
            if (grauss < 90) {
              //objeto na esquerda, ir para direita
              int tempo = time * 85;
              contagemObjeto++;

              direita(70, tempo);
              delay(200);
              if (frenteLivre() == true) {
                frente(80, 1850);
              } else {
                direita(70, 1500);
                delay(200);
                frente(80, 1850);
              }
            }

            if (grauss > 90) {
              //objeto na direita, ir para esquerda
              int tempo = time * 85;
              contagemObjeto++;
              esquerda(70, tempo);
              delay(200);
              if (frenteLivre() == true) {
                frente(80, 1850);
              } else {
                esquerda(70, 1500);
                delay(200);
                frente(80, 1850);
              }
            }
          } else if (varreduraCompleta) {
            frente(80, 200);
            varreduraCompleta = false;
          }
        }
        delay(2000);
        if (contagemObjeto == 3) {
          repeticao++;
        }
      }
    }
  }
}