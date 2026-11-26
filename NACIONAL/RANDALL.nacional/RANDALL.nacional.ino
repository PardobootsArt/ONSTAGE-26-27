//=====================================================================
//  RANDALL - versão corrigida (baseada no código mais atualizado)
//=====================================================================
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <FastLED.h>
#include <math.h>

// ======================================
// SENSOR TCS34725
// ======================================
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_1X);

const unsigned long intervaloSensor = 60;  // >= tempo de integração (50 ms)
unsigned long ultimaLeitura = 0;
unsigned long ultimoPrintCor = 0;

// ======================================
// MPU6050  (um único sistema de ângulo)
// ======================================
#define MPU_ADDR 0x68

// Positivo = virou para a ESQUERDA (igual ao seu código).
// Teste: gire o robô com a mão para a esquerda -> "ang" tem que AUMENTAR.
// Se diminuir, troque para -1.
const int SINAL_GIRO = 1;

float offsetGz = 0;
float anguloAtual = 0;     // graus, contínuo (não dá "pulo" em 180/360)
float taxaGiro = 0;        // graus/s
float rumoAlvo = 0;        // direção que o robô tem que seguir
unsigned long tempoAnteriorGyro = 0;

// ======================================
// CONTROLE DE RUMO
// ======================================
float Kp = 7.5;            // o mesmo que vocês já usavam
float Kd = 0.6;            // agora em cima da velocidade de giro (sinal certo)
const float CORRECAO_MAX = 45;

float KpCurva = 3.0;
const int VEL_MAX_CURVA = 150;     // igual ao início da sua curva
const int VEL_MIN_CURVA = 100;     // igual ao fim da sua curva
const float TOLERANCIA_CURVA = 2.0;
const unsigned long TIMEOUT_CURVA = 5000;

// ======================================
// FITA WS2811
// ======================================
#define NUM_LEDS 300
#define DATA_PIN 39
CRGB leds[NUM_LEDS];

int velocidade = 10;
CRGB corAlvo = CRGB(255, 0, 200);
int indiceLED = 0;
bool transicaoAtiva = false;
unsigned long inicioTransicao = 0;
unsigned long ultimoShow = 0;

// ======================================
// MOTORES
// ======================================
#define ENAT 11   // traseiro esquerdo
#define ENBT 10   // traseiro direito
#define INT1 22
#define INT2 24
#define INT3 26
#define INT4 28
#define INF1 30
#define INF2 32
#define INF3 34
#define INF4 36
#define ENAF 9    // dianteiro esquerdo
#define ENBF 8    // dianteiro direito

// ======================================
// VELOCIDADES E TEMPOS DA MISSÃO
// ======================================
const int velocidadeBase = 90;           // etapa 1 (procurando branco)
const int velocidadeAntesCurva = 85;     // reta depois do branco
const int velocidadeDepoisCurva = 70;    // etapa 2 (cores)

const unsigned long TEMPO_IGNORAR_BRANCO = 15000;  // substitui o "rept > 150"
const unsigned long tempoAntesDaCurva = 7300;
const float anguloCurva = 90.0;
const unsigned long tempoExtra = 2000;

// ======================================
// ULTRASSÔNICO TRASEIRO (largada)
// ======================================
#define TRIG 42
#define ECHO 40

// ======================================
// TABELA DE CORES (proporção R,G,B + brilho R+G+B)
// Tirada do meio das faixas do seu código. Recalibre com 'c' no Serial.
// ======================================
struct CorRef {
  int id;
  const char* nome;
  float r, g, b;
  float brilho;
  CRGB led;
};

CorRef tabelaCores[] = {
  { 1, "BRANCO",   0.441, 0.374, 0.185, 2837, CRGB(255, 0, 200) },
  { 2, "AMARELO",  0.533, 0.335, 0.132, 1135, CRGB(255, 120, 0) },
  { 3, "ROSA",     0.688, 0.165, 0.147,  560, CRGB(255, 0, 50) },
  { 4, "TURQUESA", 0.296, 0.487, 0.217,  473, CRGB(0, 255, 50) },
  { 5, "VERMELHO", 0.711, 0.166, 0.123,  528, CRGB(255, 0, 0) },
  { 6, "LARANJA",  0.643, 0.232, 0.125,  560, CRGB(255, 60, 0) },
  { 7, "VERDE",    0.349, 0.429, 0.222,  315, CRGB(0, 255, 0) },
  { 8, "AZUL",     0.299, 0.325, 0.377,  385, CRGB(0, 0, 255) },
  // Meça o CHÃO com 'c' e descomente, para ele nunca virar "cor":
  // { 0, "CHAO",  0.000, 0.000, 0.000,    0, CRGB(0, 0, 0) },
};
const int NUM_CORES = sizeof(tabelaCores) / sizeof(tabelaCores[0]);

const float LIMITE_COR = 0.10;
const float PESO_BRILHO = 0.15;
const int LEITURAS_CONFIRMAR = 2;

int corCandidata = -1;
int contagemCandidata = 0;

// ======================================
// MISSÃO
// LARANJA -> AZUL -> VERMELHO -> TURQUESA -> AMARELO -> ROSA -> VERDE
// ======================================
const int ordemCores[7] = { 6, 8, 5, 4, 2, 3, 7 };
int etapaAtual = 0;

bool procurandoBranco = true;
bool missaoFinalizada = false;
bool aguardandoParada = false;
unsigned long tempoFinalizacao = 0;
unsigned long inicioMissao = 0;

// ======================================
// MPU6050
// ======================================
void escreverMPU(byte registrador, byte valor) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(registrador);
  Wire.write(valor);
  Wire.endTransmission();
}

int16_t lerGz() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)2, (uint8_t) true);
  if (Wire.available() < 2) {
    return (int16_t)offsetGz;  // falha: trata como "parado"
  }
  int16_t gz = Wire.read() << 8;
  gz |= Wire.read();
  return gz;
}

void calibrarGyro() {
  long soma = 0;
  Serial.println("CALIBRANDO MPU6050...");
  Serial.println("MANTENHA O RANDALL PARADO!");
  for (int i = 0; i < 1000; i++) {
    soma += lerGz();
    delay(2);
  }
  offsetGz = soma / 1000.0;
  Serial.print("OFFSET GZ: ");
  Serial.println(offsetGz);
  Serial.println("CALIBRACAO CONCLUIDA!");
  anguloAtual = 0;
  taxaGiro = 0;
  tempoAnteriorGyro = micros();
}

// Chamada o tempo todo (não só quando lê a cor)
void atualizarAngulo() {
  int16_t gz = lerGz();
  unsigned long agora = micros();
  float dt = (agora - tempoAnteriorGyro) / 1000000.0;
  tempoAnteriorGyro = agora;
  if (dt > 0.1) dt = 0.1;

  float taxa = SINAL_GIRO * (gz - offsetGz) / 131.0;
  if (fabs(taxa) < 0.3) taxa = 0;  // zona morta: ângulo não "anda" parado

  taxaGiro = taxa;
  anguloAtual += taxa * dt;
}

// ======================================
// MOTORES
// ======================================
void acionarMotor(int in1, int in2, int en, int vel) {
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
  analogWrite(en, constrain(abs(vel), 0, 255));
}

// esq/dir: -255..255 (positivo = para frente)
void acionar(int esq, int dir) {
  acionarMotor(INF1, INF2, ENAF, esq);  // dianteiro esquerdo
  acionarMotor(INT1, INT2, ENAT, esq);  // traseiro esquerdo
  acionarMotor(INF3, INF4, ENBF, dir);  // dianteiro direito
  acionarMotor(INT3, INT4, ENBT, dir);  // traseiro direito
}

void pararMotores() {
  acionar(0, 0);
}

// Um passo de "andar reto" mantendo o rumoAlvo
void andarReto(int vel) {
  float erro = anguloAtual - rumoAlvo;      // >0 = desviou p/ esquerda
  float c = Kp * erro + Kd * taxaGiro;
  c = constrain(c, -CORRECAO_MAX, CORRECAO_MAX);
  acionar(vel + c, vel - c);                // lado esquerdo acelera -> volta p/ direita
}

// ======================================
// FUNÇÕES BLOQUEANTES (usadas só na etapa 1)
// Continuam lendo o giroscópio enquanto esperam.
// ======================================
void esperar(unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    atualizarAngulo();
    delay(2);
  }
}

void andarRetoPor(int vel, unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    atualizarAngulo();
    andarReto(vel);
    delay(2);
  }
  pararMotores();
}

// Gira até rumoAlvo + graus (positivo = esquerda)
void girar(float graus) {
  rumoAlvo += graus;
  unsigned long t0 = millis();
  int alinhado = 0;

  while (millis() - t0 < TIMEOUT_CURVA) {
    atualizarAngulo();
    float falta = rumoAlvo - anguloAtual;  // >0 = falta virar p/ esquerda

    if (fabs(falta) < TOLERANCIA_CURVA) {
      pararMotores();
      if (++alinhado > 30) break;          // ficou ~100 ms no ângulo certo
    } else {
      alinhado = 0;
      // rápido longe do alvo, devagar perto (como o seu map 150 -> 100)
      int vel = constrain(fabs(falta) * KpCurva, VEL_MIN_CURVA, VEL_MAX_CURVA);
      if (falta > 0) acionar(-vel, vel);   // esquerda p/ trás, direita p/ frente
      else acionar(vel, -vel);             // passou do ponto: volta
    }
    delay(3);
  }
  pararMotores();
  Serial.print("CURVA: ANGULO FINAL = ");
  Serial.print(anguloAtual, 1);
  Serial.print("  ALVO = ");
  Serial.println(rumoAlvo, 1);
}

// ======================================
// ULTRASSÔNICO
// ======================================
int calDistanciaT() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duracao = pulseIn(ECHO, HIGH, 25000);  // timeout: não trava 1 s
  return duracao * 0.034 / 2;
}

// ======================================
// SENSOR DE COR
// ======================================
// Lê os registradores direto, sem o delay de 50 ms do getRawData()
void lerCorRapido(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c) {
  *c = tcs.read16(TCS34725_CDATAL);
  *r = tcs.read16(TCS34725_RDATAL);
  *g = tcs.read16(TCS34725_GDATAL);
  *b = tcs.read16(TCS34725_BDATAL);
}

int classificarCor(uint16_t r, uint16_t g, uint16_t b, float* distOut) {
  float soma = (float)r + g + b;
  *distOut = 99;
  if (soma < 30) return -1;

  float pr = r / soma, pg = g / soma, pb = b / soma;
  int melhor = -1;
  float melhorDist = 99;

  for (int i = 0; i < NUM_CORES; i++) {
    CorRef& ref = tabelaCores[i];
    if (ref.brilho <= 0) continue;
    float d = fabs(pr - ref.r) + fabs(pg - ref.g) + fabs(pb - ref.b)
              + PESO_BRILHO * fabs(log(soma / ref.brilho));
    if (d < melhorDist) {
      melhorDist = d;
      melhor = i;
    }
  }
  *distOut = melhorDist;
  if (melhorDist > LIMITE_COR) return -1;
  return melhor;
}

// Retorna o id da cor confirmada (uma vez por passagem) ou 0
int verificarCor() {
  if (millis() - ultimaLeitura < intervaloSensor) return 0;
  ultimaLeitura = millis();

  uint16_t r, g, b, c;
  lerCorRapido(&r, &g, &b, &c);

  float dist;
  int idx = classificarCor(r, g, b, &dist);

  if (millis() - ultimoPrintCor > 150) {
    ultimoPrintCor = millis();
    Serial.print("R: "); Serial.print(r);
    Serial.print(" | G: "); Serial.print(g);
    Serial.print(" | B: "); Serial.print(b);
    Serial.print(" | cor: ");
    Serial.print(idx >= 0 ? tabelaCores[idx].nome : "-");
    Serial.print(" (dist "); Serial.print(dist, 3);
    Serial.print(") | ang: "); Serial.println(anguloAtual, 1);
  }

  if (idx >= 0 && idx == corCandidata) {
    contagemCandidata++;
  } else {
    corCandidata = idx;
    contagemCandidata = 1;
  }

  if (idx >= 0 && contagemCandidata == LEITURAS_CONFIRMAR) {
    return tabelaCores[idx].id;
  }
  return 0;
}

void resetarConfirmacao() {
  corCandidata = -1;
  contagemCandidata = 0;
}

int indicePorId(int id) {
  for (int i = 0; i < NUM_CORES; i++)
    if (tabelaCores[i].id == id) return i;
  return -1;
}

// Comando 'c' no Serial: média de 20 leituras no formato da tabela
void calibrarCor() {
  pararMotores();
  Serial.println("CALIBRANDO COR... (robo parado sobre a cor)");
  float sr = 0, sg = 0, sb = 0;
  for (int i = 0; i < 20; i++) {
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    sr += r; sg += g; sb += b;
  }
  sr /= 20; sg /= 20; sb /= 20;
  float soma = sr + sg + sb;
  if (soma < 1) soma = 1;
  Serial.print("COPIE PARA A TABELA:  { id, \"NOME\", ");
  Serial.print(sr / soma, 3); Serial.print(", ");
  Serial.print(sg / soma, 3); Serial.print(", ");
  Serial.print(sb / soma, 3); Serial.print(", ");
  Serial.print(soma, 0);
  Serial.println(", CRGB(...) },");
  tempoAnteriorGyro = micros();
}

// ======================================
// FITA LED (sem travar o loop)
// ======================================
void iniciarTransicao(CRGB cor) {
  corAlvo = cor;
  indiceLED = 0;
  inicioTransicao = millis();
  transicaoAtiva = true;
}

void atualizarLeds() {
  if (!transicaoAtiva) return;
  // show() de 300 LEDs trava as interrupções ~9 ms; atualiza no máx. a cada 40 ms
  if (millis() - ultimoShow < 40) return;
  ultimoShow = millis();

  int alvo = (millis() - inicioTransicao) / velocidade + 1;
  if (alvo > NUM_LEDS) alvo = NUM_LEDS;
  while (indiceLED < alvo) {
    leds[indiceLED++] = corAlvo;
  }
  FastLED.show();

  if (indiceLED >= NUM_LEDS) {
    transicaoAtiva = false;
    Serial.println("TRANSICAO CONCLUIDA!");
  }
}

// ======================================
// SEQUÊNCIA DEPOIS DO BRANCO
// ======================================
void sequenciaCurva() {
  Serial.println("-------------------------");
  Serial.println("BRANCO IDENTIFICADO!");
  Serial.println("ANDANDO ANTES DA CURVA...");

  pararMotores();
  esperar(500);

  andarRetoPor(velocidadeAntesCurva, tempoAntesDaCurva);  // agora COM correção

  esperar(1000);
  girar(anguloCurva);   // o rumoAlvo já fica 90° à esquerda do anterior
  esperar(1000);

  procurandoBranco = false;
  resetarConfirmacao();

  Serial.println("-------------------------");
  Serial.println("CURVA FINALIZADA!");
  Serial.println("ETAPA 2: PROCURANDO AS 7 CORES.");
}

// ======================================
// SETUP
// ======================================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  pinMode(ENAT, OUTPUT);
  pinMode(ENBT, OUTPUT);
  pinMode(ENAF, OUTPUT);
  pinMode(ENBF, OUTPUT);
  pinMode(INT1, OUTPUT);
  pinMode(INT2, OUTPUT);
  pinMode(INT3, OUTPUT);
  pinMode(INT4, OUTPUT);
  pinMode(INF1, OUTPUT);
  pinMode(INF2, OUTPUT);
  pinMode(INF3, OUTPUT);
  pinMode(INF4, OUTPUT);
  pararMotores();

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // MPU6050
  escreverMPU(0x6B, 0x00);
  escreverMPU(0x1A, 0x03);  // filtro ~44 Hz contra vibração
  escreverMPU(0x1B, 0x00);
  delay(100);

  if (!tcs.begin()) {
    Serial.println("ERRO: TCS34725 NAO ENCONTRADO!");
    while (1);
  }

  FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 200));
  FastLED.show();
  Serial.println("-------------------------");
  Serial.println("FITA LED INICIADA EM ROXO.");

  calibrarGyro();

  // ESPERA A LARGADA: algo a menos de 5 cm atrás
  Serial.println("AGUARDANDO LARGADA (mao atras do robo)...");
  while (true) {
    atualizarAngulo();
    int distancia = calDistanciaT();
    if (distancia > 0 && distancia < 5) break;
    esperar(30);
  }

  esperar(300);              // deixa assentar se alguém encostou no robô
  rumoAlvo = anguloAtual;    // direção da largada = direção a manter
  inicioMissao = millis();

  Serial.println("MISSAO INICIADA!");
  Serial.println("ETAPA 1: PROCURANDO BRANCO.");
}

// ======================================
// LOOP
// ======================================
void loop() {
  atualizarAngulo();

  if (Serial.available()) {
    char comando = Serial.read();
    if (comando == '1') velocidade = 10;
    else if (comando == '2') velocidade = 20;
    else if (comando == '3') velocidade = 30;
    else if (comando == '4') velocidade = 40;
    else if (comando == '5') velocidade = 60;
    else if (comando == '6') velocidade = 80;
    else if (comando == '7') velocidade = 100;
    else if (comando == 'c') calibrarCor();

    if (comando >= '1' && comando <= '7') {
      Serial.print("VELOCIDADE DOS LEDS: ");
      Serial.print(velocidade);
      Serial.println(" ms por LED");
    }
  }

  // ---------- ETAPA 1: PROCURAR BRANCO ----------
  if (procurandoBranco) {
    andarReto(velocidadeBase);
    int cor = verificarCor();
    if (cor == 1 && millis() - inicioMissao > TEMPO_IGNORAR_BRANCO) {
      sequenciaCurva();
    }
  }

  // ---------- ETAPA 2: PROCURAR AS 7 CORES ----------
  else if (!missaoFinalizada) {
    andarReto(velocidadeDepoisCurva);

    if (!aguardandoParada) {
      int cor = verificarCor();
      if (cor != 0 && etapaAtual < 7 && cor == ordemCores[etapaAtual]) {
        int idx = indicePorId(cor);
        Serial.println("-------------------------");
        Serial.print("NOVA COR IDENTIFICADA: ");
        Serial.println(tabelaCores[idx].nome);

        iniciarTransicao(tabelaCores[idx].led);
        etapaAtual++;

        Serial.print("PROGRESSO: ");
        Serial.print(etapaAtual);
        Serial.println("/7");

        if (etapaAtual >= 7) {
          aguardandoParada = true;
          tempoFinalizacao = millis();
          Serial.println("7 CORES IDENTIFICADAS! RANDALL CONTINUARA ANDANDO.");
        }
      }
    }

    if (aguardandoParada && millis() - tempoFinalizacao >= tempoExtra) {
      pararMotores();
      aguardandoParada = false;
      missaoFinalizada = true;
      Serial.println("MISSAO CONCLUIDA! MOTORES PARADOS.");
    }
  }

  else {
    pararMotores();
  }

  atualizarLeds();
}

/*=====================================================================
  COMO TESTAR
  1) Gire o robô com a mão para a ESQUERDA: "ang" tem que aumentar.
     Se diminuir, SINAL_GIRO = -1.
  2) Curva passando do ponto: diminua VEL_MAX_CURVA ou KpCurva.
     Parando antes e "zumbindo": aumente VEL_MIN_CURVA.
  3) Cores: sensor sobre a cor na altura real, mande 'c', copie a linha.
     Faça o mesmo com o chão e descomente a linha CHAO.
  4) TEMPO_IGNORAR_BRANCO = 15 s equivale ao seu "rept > 150". Ajuste.
=====================================================================*/