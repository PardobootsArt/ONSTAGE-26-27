//giroscopio
#include <Wire.h>
const int MPU = 0x68;

float anguloZ = 0.0;
unsigned long ultimoTempo;

float offsetGz = 0;

bool iniciarSullivan = false;
//=============MOVIMENTAÇÃO=============
#define bracoE1 49
#define bracoE2 47
#define bracoD1 51
#define bracoD2 53
#define enbracoD 7
#define enbracoE 8
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
bool sentidoHorario = false;
int contagemObjeto;
int repeticao;

//===========CORREÇÃO===========
float Kp = 1.6;
float Kd = 0.1;
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

    int oi = (posicaoAtual * (-2)) + 200;

    girarTronco(125, oi);

    delay(15);

    int distancia = lerDistancia();

    if (distancia > 0 && distancia < 30) {

      delay(150);

      varreduraCompleta = false;

      return posicaoAtual;
    }

    if (sentidoHorario == false) {

      posicaoAtual -= 4;

      if (posicaoAtual <= 25) {

        posicaoAtual = 25;

        sentidoHorario = true;

        varreduraCompleta = true;
      }

    } else if (sentidoHorario == true) {

      posicaoAtual += 4;

      if (posicaoAtual >= 145) {

        posicaoAtual = 155;

        sentidoHorario = false;
      }
    }
  }

  return -1;
}

void radar() {

  int repeticoes;

  while (repeticoes < 4) {

    meuServo.write(posicaoAtual);

    int oi = (posicaoAtual * (-2)) + 200;

    girarTronco(120, oi);

    delay(15);

    if (sentidoHorario == false) {

      posicaoAtual -= 4;

      if (posicaoAtual <= 25) {

        posicaoAtual = 25;

        sentidoHorario = true;

        varreduraCompleta = true;
      }

    } else if (sentidoHorario == true) {

      posicaoAtual += 4;

      if (posicaoAtual >= 145) {

        posicaoAtual = 155;

        sentidoHorario = false;

        repeticoes++;
      }
    }
  }
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
    Wire.write(0x47);
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

//===============================================
// SETUP
//===============================================

void setup() {

  // COMUNICAÇÃO
  Serial.begin(115200);
  Serial2.begin(9600);

  // Comunicação com a ESP B

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

  attachInterrupt(
    digitalPinToInterrupt(yTronco),
    countDegrees,
    CHANGE);

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

  analogWrite(enbtronco, 175);

  delay(150);
}

//===============================================
// CORREÇÃO
//===============================================

float correcao() {

  float ang = obterAngulo();

  float erro = anguloReferencia - ang;

  unsigned long agora = millis();

  float dt = (agora - ultimoControle) / 1000.0;

  if (dt <= 0)
    dt = 0.001;

  float deriv = (erro - ultimoErro) / dt;

  ultimoErro = erro;
  ultimoControle = agora;

  return Kp * erro + Kd * deriv;
}

//===============================================
// PARAR MOTORES
//===============================================

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

//===============================================
// ESQUERDA
//===============================================

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

//===============================================
// DIREITA
//===============================================

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

//===============================================
// FRENTE
//===============================================

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

//===============================================
// TRÁS
//===============================================

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

//===============================================
// CALCULAR TEMPO
//===============================================

int calcularTempo(int angulo) {

  int erro = abs(angulo - 90);

  float fator = cos(radians(erro));

  int tempo = 50 * fator;

  tempo = constrain(tempo, 0, 500);

  return tempo;
}

//===============================================
// FRENTE LIVRE
//===============================================

bool frenteLivre() {

  meuServo.write(90);

  delay(80);

  int distancia = lerDistancia();

  if (distancia > 20 || distancia == 0) {

    return true;
  }

  return false;
}

//===============================================
// LOOP
//===============================================

void loop() {
  // ==========================
  // ESP B -> Sullivan
  // ==========================

  if (!iniciarSullivan && Serial2.available()) {

    String mensagem = Serial2.readStringUntil('\n');
    mensagem.trim();

    Serial.print("Recebido da ESP B: ");
    Serial.println(mensagem);

    if (mensagem == "1") {

      iniciarSullivan = true;

      Serial.println("SINAL RECEBIDO!");
      Serial.println("SULLIVAN INICIANDO!");
      if (repeticao == 0) {

        repeticao++;
        delay(2500);

        direita(150, 13150);

        pararMotores();

        delay(500);

        frente(150, 3250);

        delay(1000);

        // PROCURA OS 2 OBJETOS

        while (contagemObjeto < 4) {

          int grauss = procurarObjeto();

          if (grauss != -1) {

            int tempo = (calcularTempo(grauss)) * 70;

            if (grauss > 90) {

              // Objeto na esquerda -> virar para a direita

              contagemObjeto++;

              direita(150, tempo);

              pararMotores();

              delay(200);

              Serial.println(tempo);

              if (frenteLivre() == true) {
                if (contagemObjeto == 4) {

                  frente(150, 3400);
                } else {
                  frente(150, 5200);
                }

                pararMotores();
              }
            }

            else if (grauss < 90) {

              // Objeto na direita -> virar para a esquerda

              contagemObjeto++;

              esquerda(150, tempo);

              pararMotores();

              delay(200);

              Serial.println(tempo);

              if (frenteLivre() == true) {

                if (contagemObjeto == 4) {

                  frente(150, 3400);
                } else {
                  frente(150, 5200);
                }
              }
            }
          }

          else if (varreduraCompleta) {

            frente(110, 200);

            varreduraCompleta = false;
          }
        }

        //==========================================
        // ENCONTROU OS 2 OBJETOS
        //==========================================

        bool levantarBracos = false;
        while (contagemObjeto > 1) {
          Serial.println("entrei");
          int distancia = lerDistancia();
          if (distancia > 0 && distancia < 15) {
            Serial.println("levantei");
            analogWrite(enbracoE, 120);
            analogWrite(enbracoD, 120);
            digitalWrite(bracoE1, HIGH);
            digitalWrite(bracoE2, LOW);
            digitalWrite(bracoD1, HIGH);
            digitalWrite(bracoD2, LOW);
            delay(450);
            digitalWrite(bracoE1, LOW);
            digitalWrite(bracoE2, LOW);
            delay(50);
            digitalWrite(bracoD1, LOW);
            digitalWrite(bracoD2, LOW);
          }
          //==========================================
          // LEVANTA OS DOIS BRAÇOS
          //==========================================

          // Não inicia novamente
          repeticao = 1;
        }
      }

      else {
        // Sem objeto: permanece parado.
        pararMotores();
      }
    }
  }

  // Enquanto não receber o sinal
  if (!iniciarSullivan) {

    pararMotores();  // ou sua função equivalente

    return;
  }
}