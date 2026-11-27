//=====================================================================
//  MIKE - segue a AprilTag GIRANDO para alinhar (sem andar de lado)
//  Mensagem do Python:  tagID,erroX,erroDistancia,PosicaoOlho\n
//
//  Tag à ESQUERDA da câmera -> motores da DIREITA mais fortes (vira p/ esquerda)
//  Tag à DIREITA da câmera  -> motores da ESQUERDA mais fortes (vira p/ direita)
//=====================================================================
#include <Servo.h>

//olho
Servo horizontal;
Servo vertical;

//braços
Servo direito;
Servo esquerdo;

void movimentacaoolho();
void olho(int vertical2, int horizontal2);
void girar(int lado, unsigned long tempo);

//=====================================================
// VARIAVEIS RECEBIDAS DO PYTHON
//=====================================================
int tagID = -1;
float erroX = 0;           // >0 = tag à direita, <0 = tag à esquerda
float erroDistancia = 0;
int PosicaoOlho = 0;

//=====================================================
// AJUSTES DO CONTROLE
//=====================================================
float KpDist = 0.6;        // distância -> PWM para frente
const int VEL_BASE = 40;   // se os motores mal se mexerem, teste 60~80

float KpCurva = 0.1;       // pixels -> diferença de PWM entre os lados
float KdCurva = 0.02;       // amortecimento (use 0.02~0.05 se ficar "serpenteando")
const float CURVA_MAX = 80;          // diferença máxima entre os lados
const float ZONA_MORTA_X = 15;       // até 15 px do centro = anda reto

// Se com a tag à esquerda ele virar para a DIREITA, troque para true
const bool INVERTER_CURVA = false;

const unsigned long TIMEOUT_DADOS = 500;

// Ajuste fino de cada motor (lado mais forte -> diminua, ex: 0.90)
float fatorEF = 1.00;   // esquerdo frente
float fatorET = 1.00;   // esquerdo trás
float fatorDF = 1.00;   // direito frente
float fatorDT = 1.00;   // direito trás

//=====================================================
// PORTAS MOTORES
//=====================================================
int motorDF1 = 22;
int motorDF2 = 24;
int motorEF1 = 26;
int motorEF2 = 28;
int motorDT1 = 30;
int motorDT2 = 32;
int motorET1 = 34;
int motorET2 = 36;

int enbEF = 12;
int enbDF = 9;   // Se mudou o fio para o pino 9, troque aqui para 9
int enbET = 11;
int enbDT = 10;

bool repBraco1 = false;
bool repTag2 = false;
bool repTag4 = false;
bool repTag5 = false;

//=====================================================
// LEITURA DA SERIAL (sem travar, sempre a mensagem mais nova)
//=====================================================
String bufferSerial = "";
String ultimaMensagem = "";
bool novaMensagem = false;
unsigned long ultimoDado = 0;
bool motoresParados = true;

float erroXAnterior = 0;
unsigned long tempoErroAnterior = 0;

void lerSerial() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') {
      if (bufferSerial.length() > 0) {
        ultimaMensagem = bufferSerial;
        novaMensagem = true;
        bufferSerial = "";
      }
    } else if (bufferSerial.length() < 60) {
      bufferSerial += ch;
    }
  }
}

void limparSerial() {
  while (Serial.available()) Serial.read();
  bufferSerial = "";
  novaMensagem = false;
}

bool interpretarMensagem(String mensagem) {
  mensagem.trim();
  int v1 = mensagem.indexOf(',');
  int v2 = mensagem.indexOf(',', v1 + 1);
  int v3 = mensagem.indexOf(',', v2 + 1);
  if (v1 == -1 || v2 == -1 || v3 == -1) return false;

  tagID = mensagem.substring(0, v1).toInt();
  erroX = mensagem.substring(v1 + 1, v2).toFloat();
  erroDistancia = mensagem.substring(v2 + 1, v3).toFloat();
  PosicaoOlho = mensagem.substring(v3 + 1).toInt();
  return true;
}

//=====================================================
// MOTORES
//=====================================================
// vel positivo = sentido "para a tag" (o mesmo do código antigo)
void acionarMotor(int pinoFrente, int pinoTras, int en, float vel) {
  if (vel > 0) {
    digitalWrite(pinoFrente, HIGH);
    digitalWrite(pinoTras, LOW);
  } else if (vel < 0) {
    digitalWrite(pinoFrente, LOW);
    digitalWrite(pinoTras, HIGH);
  } else {
    digitalWrite(pinoFrente, LOW);
    digitalWrite(pinoTras, LOW);
  }
  analogWrite(en, constrain((int)fabs(vel), 0, 255));
}

// Lado esquerdo (EF + ET) e lado direito (DF + DT) juntos, como um tanque
void moverLados(float velEsq, float velDir) {
  float velEF = velEsq * fatorEF;
  float velET = velEsq * fatorET;
  float velDF = velDir * fatorDF;
  float velDT = velDir * fatorDT;

  // se passar de 255, reduz todos na mesma proporção (mantém a curva)
  float maior = max(max(fabs(velEF), fabs(velET)), max(fabs(velDF), fabs(velDT)));
  if (maior > 255) {
    float k = 255.0 / maior;
    velEF *= k; velET *= k; velDF *= k; velDT *= k;
  }

  acionarMotor(motorEF1, motorEF2, enbEF, velEF);
  acionarMotor(motorET2, motorET1, enbET, velET);
  acionarMotor(motorDF2, motorDF1, enbDF, velDF);
  acionarMotor(motorDT2, motorDT1, enbDT, velDT);
  motoresParados = false;
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
  motoresParados = true;
}

//=====================================================
// SEGUIR A TAG (tagID == 0)
//=====================================================
void seguirTag() {
  movimentacaoolho();

  // Velocidade para frente (aproxima da tag)
  float vy = VEL_BASE + KpDist * erroDistancia;
  vy = constrain(vy, 0, 255);

  // Diferença entre os lados (curva)
  float c = 0;
  if (fabs(erroX) > ZONA_MORTA_X) {
    c = KpCurva * erroX;
  }

  // Amortecimento: se a tag está voltando rápido para o centro,
  // alivia a curva antes de passar do ponto
  unsigned long agora = millis();
  if (tempoErroAnterior != 0 && KdCurva > 0) {
    float dt = (agora - tempoErroAnterior) / 1000.0;
    if (dt > 0.001 && dt < 0.5) {
      c += KdCurva * (erroX - erroXAnterior) / dt;
    }
  }
  erroXAnterior = erroX;
  tempoErroAnterior = agora;

  c = constrain(c, -CURVA_MAX, CURVA_MAX);
  if (INVERTER_CURVA) c = -c;

  // Tag à direita (c > 0): esquerda mais forte -> vira p/ direita
  // Tag à esquerda (c < 0): direita mais forte -> vira p/ esquerda
  // Se a curva for maior que vy, o lado de dentro gira para trás
  // e o robô vira no próprio lugar até alinhar.
  float velEsq = vy + c;
  float velDir = vy - c;

  moverLados(velEsq, velDir);
}

//=====================================================
// SETUP
//=====================================================
void setup() {
  Serial.begin(9600);

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

  horizontal.attach(3);
  vertical.attach(2);
}

//=====================================================
// BRAÇOS
//=====================================================
void falarOi() {
  direito.attach(4);
  esquerdo.attach(5);
  esquerdo.write(180);
  direito.write(0);

  for (int i = 0; i < 3; i++) {
    direito.write(120);
    delay(500);
    direito.write(180);
    delay(500);
  }
  direito.write(0);
}

void animado() {
  if (repBraco1 == false) {
    repBraco1 = true;
    direito.attach(4);
    esquerdo.attach(5);
    esquerdo.write(180);
    delay(150);
    direito.write(0);
    delay(500);
    direito.write(180);
    delay(150);
    esquerdo.write(0);
    delay(500);
    direito.write(90);
    delay(150);
    esquerdo.write(90);
  }
}

void comemorar() {
  direito.attach(4);
  esquerdo.attach(5);
  esquerdo.write(180);
  delay(150);
  direito.write(0);
  delay(500);

  for (int i = 0; i < 3; i++) {
    direito.write(180);
    delay(150);
    esquerdo.write(0);
    delay(500);
    direito.write(90);
    delay(150);
    esquerdo.write(90);
    delay(500);
  }

  esquerdo.write(180);
  delay(150);
  direito.write(0);
}

// lado = 1 -> direita | lado = -1 -> esquerda (pinos do código antigo)
void girar(int lado, unsigned long tempo) {
  if (lado == 1) {
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);
    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
  } else if (lado == -1) {
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
  }
  analogWrite(enbEF, 100);
  analogWrite(enbDF, 100);
  analogWrite(enbET, 100);
  analogWrite(enbDT, 100);

  delay(tempo);

  pararMotores();
}

void retomarDepoisDeAcao() {
  tempoErroAnterior = 0;
  limparSerial();
}

//=====================================================
// LOOP
//=====================================================
void loop() {
  lerSerial();

  if (novaMensagem) {
    novaMensagem = false;

    if (interpretarMensagem(ultimaMensagem)) {
      ultimoDado = millis();

      if (tagID == 0) {
        repTag2 = false;
        repTag4 = false;
        repTag5 = false;
        seguirTag();
      }

      else if (tagID == -1) {
        pararMotores();
        tempoErroAnterior = 0;
      }

      else if (tagID == 2 && repTag2 == false) {
        repTag2 = true;
        pararMotores();
        girar(-1, 1300);
        delay(500);
        retomarDepoisDeAcao();
      }

      else if (tagID == 3) {
        movimentacaoolho();
      }

      else if (tagID == 4 && repTag4 == false) {
        repTag4 = true;
        pararMotores();
        falarOi();
        delay(500);
        retomarDepoisDeAcao();
      }

      else if (tagID == 5 && repTag5 == false) {
        repTag5 = true;
        pararMotores();
        comemorar();
        retomarDepoisDeAcao();
      }
    }
  }

  // Segurança: sem dados do Python -> para
  if (!motoresParados && millis() - ultimoDado > TIMEOUT_DADOS) {
    pararMotores();
    tempoErroAnterior = 0;
  }
}

//=====================================================
// OLHO
//=====================================================
void olho(int vertical2, int horizontal2) {
  vertical.write(vertical2);
  horizontal.write(horizontal2);
}

void movimentacaoolho() {
  switch (PosicaoOlho) {
    case 1: olho(0, 90);  break;
    case 2: olho(45, 90); break;
    case 3: olho(90, 90); break;
    case 4: olho(0, 50);  break;
    case 5: olho(45, 50); break;
    case 6: olho(90, 50); break;
    case 7: olho(0, 30);  break;
    case 8: olho(45, 30); break;
    case 9: olho(90, 30); break;
  }
}