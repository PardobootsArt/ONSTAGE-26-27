#include <Wire.h>
#include <Servo.h>

//============= GIROSCÓPIO =============
const int MPU = 0x68;
const int SINAL_GIRO = +1;

float anguloZ = 0.0;      // ângulo acumulado (graus, positivo = virou p/ direita)
float taxaZ = 0.0;        // velocidade de giro atual (graus/s)
float offsetGz = 0;
unsigned long ultimoTempo;
float rumoAlvo = 0;       // direção que o robô deve manter o tempo todo

//============= CONTROLES  =============
float Kp = 4.0;          
float Kd = 0.8;          
const float W_MAX = 90;   
const unsigned long RAMPA_MS = 300;  
const int PWM_MIN_GIRO = 70;         

float fatorEF = 1.00;
float fatorDF = 1.00;
float fatorET = 1.00;
float fatorDT = 1.00;

bool iniciarSullivan = false;

//============= MOVIMENTAÇÃO (braços/tronco) =============
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

//============= RADAR =============
Servo meuServo;
#define TRIG 5
#define ECHO 4

int posicaoAtual = 25;
bool sentidoHorario = false;
int contagemObjeto;
int repeticao;
bool varreduraCompleta = false;

//============= MOTORES =============
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

//=====================================================================
// GIROSCÓPIO
//=====================================================================
int16_t lerGzBruto() {
  Wire.beginTransmission(MPU);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 2, true);
  if (Wire.available() < 2) return (int16_t)offsetGz;  
  int16_t gz = Wire.read() << 8 | Wire.read();
  return gz;
}

void iniciarMPU() {
  Wire.begin();
  Wire.setWireTimeout(3000, true);  

  Wire.beginTransmission(MPU);  
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU);  
  Wire.write(0x1A);
  Wire.write(0x03);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU); 
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(100);

  
  long soma = 0;
  for (int i = 0; i < 1000; i++) {
    soma += lerGzBruto();
    delay(2);
  }
  offsetGz = soma / 1000.0;

  anguloZ = 0;
  taxaZ = 0;
  ultimoTempo = micros();
}

// Chamar com frequência (a cada poucos ms). Integra o giro de forma contínua.
void atualizarGiro() {
  int16_t gz = lerGzBruto();

  unsigned long agora = micros();
  float dt = (agora - ultimoTempo) / 1000000.0;
  ultimoTempo = agora;
  if (dt > 0.1) dt = 0.1; 

  float taxa = SINAL_GIRO * (gz - offsetGz) / 131.0;
  if (fabs(taxa) < 0.3) taxa = 0;  

  taxaZ = taxa;
  anguloZ += taxa * dt;
}

float obterAngulo() {
  atualizarGiro();
  return anguloZ;
}

// delay() que continua lendo o giroscópio
void esperar(unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    atualizarGiro();
    delay(2);
  }
}

//=====================================================================
// TRONCO
//=====================================================================
void countDegrees() {
  if (digitalRead(yTronco) == digitalRead(bTronco)) {
    degrees++;
  } else {
    degrees--;
  }
}

long lerGraus() {  // leitura segura de variável alterada por interrupção
  noInterrupts();
  long g = degrees;
  interrupts();
  return g;
}

void girarTronco(int speed, int anguloo) {
  analogWrite(enbtronco, speed);
  unsigned long t0 = millis();

  if (anguloo > lerGraus()) {
    digitalWrite(tronco1, HIGH);
    digitalWrite(tronco2, LOW);
    while (lerGraus() < anguloo && millis() - t0 < 2000) {
      atualizarGiro();
    }
  } else if (anguloo < lerGraus()) {
    digitalWrite(tronco1, LOW);
    digitalWrite(tronco2, HIGH);
    while (lerGraus() > anguloo && millis() - t0 < 2000) {
      atualizarGiro();
    }
  }

  digitalWrite(tronco1, LOW);
  digitalWrite(tronco2, LOW);
}

//=====================================================================
// RADAR
//=====================================================================
int lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // timeout de 25 ms (~4 m). Sem isso o pulseIn podia travar 1 segundo inteiro.
  long duracao = pulseIn(ECHO, HIGH, 25000);
  return duracao * 0.034 / 2;
}

int procurarObjeto() {
  while (true) {
    meuServo.write(posicaoAtual);
    int oi = (posicaoAtual * (-2)) + 200;
    girarTronco(125, oi);
    esperar(15);

    int distancia = lerDistancia();
    if (distancia > 0 && distancia < 30) {
      esperar(150);
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
    } else {
      posicaoAtual += 4;
      if (posicaoAtual >= 155) {
        posicaoAtual = 155;
        sentidoHorario = false;
      }
    }
  }
  return -1;
}

void radar() {
  int repeticoes = 0;  // antes não era inicializada

  while (repeticoes < 4) {
    meuServo.write(posicaoAtual);
    int oi = (posicaoAtual * (-2)) + 200;
    girarTronco(120, oi);
    esperar(15);

    if (sentidoHorario == false) {
      posicaoAtual -= 4;
      if (posicaoAtual <= 25) {
        posicaoAtual = 25;
        sentidoHorario = true;
        varreduraCompleta = true;
      }
    } else {
      posicaoAtual += 4;
      if (posicaoAtual >= 155) {
        posicaoAtual = 155;
        sentidoHorario = false;
        repeticoes++;
      }
    }
  }
}

//=====================================================================
// MOTORES
//=====================================================================
// vel: -255..255 (positivo = roda para FRENTE)
void acionarMotor(int in1, int in2, int en, float vel, bool invertido) {
  if (invertido) vel = -vel;
  int pwm = constrain((int)fabs(vel), 0, 255);

  if (vel > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (vel < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  analogWrite(en, pwm);
}

//   vy = frente(+)/trás(-)   vx = direita(+)/esquerda(-)   w = girar p/ direita(+)
void aplicarMecanum(float vy, float vx, float w) {
  float fl = (vy + vx + w) * fatorEF;
  float fr = (vy - vx - w) * fatorDF;
  float rl = (vy - vx + w) * fatorET;
  float rr = (vy + vx - w) * fatorDT;

  float maior = max(max(fabs(fl), fabs(fr)), max(fabs(rl), fabs(rr)));
  if (maior > 255) {
    float k = 255.0 / maior;
    fl *= k; fr *= k; rl *= k; rr *= k;
  }

  acionarMotor(motorEF1, motorEF2, enbEF, fl, true);   // EF frente = LOW/HIGH
  acionarMotor(motorDF1, motorDF2, enbDF, fr, false);  // DF frente = HIGH/LOW
  acionarMotor(motorET1, motorET2, enbET, rl, true);   // ET frente = LOW/HIGH
  acionarMotor(motorDT1, motorDT2, enbDT, rr, false);  // DT frente = HIGH/LOW
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

//=====================================================================
// CONTROLE DE RUMO
//=====================================================================
float calcularCorrecao() {
  float erro = anguloZ - rumoAlvo;      
  float w = -(Kp * erro + Kd * taxaZ);
  return constrain(w, -W_MAX, W_MAX);
}


void alinhar(unsigned long timeoutMs) {
  unsigned long t0 = millis();
  int estavel = 0;

  while (millis() - t0 < timeoutMs) {
    atualizarGiro();
    float erro = anguloZ - rumoAlvo;

    if (fabs(erro) < 1.5) {
      pararMotores();
      if (++estavel > 30) break;  
    } else {
      estavel = 0;
      float w = -(Kp * erro + Kd * taxaZ);
      if (w > 0 && w < PWM_MIN_GIRO) w = PWM_MIN_GIRO;
      if (w < 0 && w > -PWM_MIN_GIRO) w = -PWM_MIN_GIRO;
      w = constrain(w, -120, 120);
      aplicarMecanum(0, 0, w);
    }
    delay(3);
  }
  pararMotores();
}

void mover(float vy, float vx, unsigned long tempoMs) {
  alinhar(1500);

  unsigned long inicio = millis();
  unsigned long ultimoPrint = 0;

  while (millis() - inicio < tempoMs) {
    atualizarGiro();
    float w = calcularCorrecao();

    unsigned long t = millis() - inicio;
    float r = (t < RAMPA_MS) ? (float)t / RAMPA_MS : 1.0;

    aplicarMecanum(vy * r, vx * r, w);

    if (millis() - ultimoPrint > 100) {  // print a cada 100 ms (não atrasa o controle)
      ultimoPrint = millis();
      Serial.print("ang: ");
      Serial.print(anguloZ);
      Serial.print("  taxa: ");
      Serial.print(taxaZ);
      Serial.print("  w: ");
      Serial.println(w);
    }
    delay(3);
  }

  pararMotores();
  esperar(150);
}

void frente(int velo, int tempoMs)   { mover(velo, 0, tempoMs); }
void tras(int velo, int tempoMs)     { mover(-velo, 0, tempoMs); }
void esquerda(int velo, int tempoMs) { mover(0, -velo, tempoMs); } 
void direita(int velo, int tempoMs)  { mover(0, velo, tempoMs); }   

//=====================================================================
// CALCULAR TEMPO
//=====================================================================
int calcularTempo(int angulo) {
  int erro = abs(angulo - 90);
  float fator = cos(radians(erro));
  int tempo = 50 * fator;
  tempo = constrain(tempo, 0, 500);
  return tempo;
}

//=====================================================================
// FRENTE LIVRE
//=====================================================================
bool frenteLivre() {
  meuServo.write(90);
  esperar(80);
  int distancia = lerDistancia();
  return (distancia > 20 || distancia == 0);
}

//=====================================================================
// SETUP
//=====================================================================
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);  // Comunicação com a ESP B

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
  pararMotores();

  pinMode(bracoE1, OUTPUT);
  pinMode(bracoE2, OUTPUT);
  pinMode(bracoD1, OUTPUT);
  pinMode(bracoD2, OUTPUT);
  pinMode(enbracoD, OUTPUT);
  pinMode(enbracoE, OUTPUT);

  pinMode(tronco1, OUTPUT);
  pinMode(tronco2, OUTPUT);
  pinMode(enbtronco, OUTPUT);
  digitalWrite(tronco1, LOW);
  digitalWrite(tronco2, LOW);

  pinMode(yTronco, INPUT);
  pinMode(bTronco, INPUT);
  degrees = 0;
  attachInterrupt(digitalPinToInterrupt(yTronco), countDegrees, CHANGE);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  meuServo.attach(servo);
  meuServo.write(90);
  delay(500);

  // Calibra com tudo parado
  iniciarMPU();
  rumoAlvo = anguloZ;
  Serial.println("MPU pronto!");

  pinMode(20, INPUT);
  pinMode(21, INPUT);

  contagemObjeto = 0;
  repeticao = 0;

  analogWrite(enbtronco, 175);
  esperar(150);
}

//=====================================================================
// LOOP
//=====================================================================
void loop() {
  atualizarGiro();

  int dist = lerDistancia();
  if (dist > 0 && dist < 30) {
    while (contagemObjeto < 2) {
      int grauss = procurarObjeto();

      if (grauss != -1) {
        int tempo = (calcularTempo(grauss)) * 55;

        if (grauss > 90) {
          // Objeto na esquerda -> desviar para a direita
          contagemObjeto++;
          direita(150, tempo);
          Serial.println(tempo);

          if (frenteLivre()) {
            frente(150, 8500);
          }
        } else if (grauss < 90) {
          // Objeto na direita -> desviar para a esquerda
          contagemObjeto++;
          esquerda(150, tempo);
          Serial.println(tempo);

          if (frenteLivre()) {
            frente(150, 8500);
          }
        }
      } else if (varreduraCompleta) {
        frente(110, 200);
        varreduraCompleta = false;
      }
    }

    //==========================================
    // ENCONTROU OS 2 OBJETOS -> LEVANTA OS BRAÇOS
    //==========================================
    while (contagemObjeto > 1) {
      Serial.println("entrei");
      int distancia = lerDistancia();
      if (distancia > 0 && distancia < 30) {
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
      repeticao = 1;  // Não inicia novamente
    }
  }
}