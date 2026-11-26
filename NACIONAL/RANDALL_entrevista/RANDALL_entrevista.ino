#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <FastLED.h>
#include <math.h>

// ======================================
// SENSOR DE COR TCS34725
// ======================================
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_1X);

const unsigned long intervaloSensor = 60;
unsigned long ultimaLeitura = 0;
unsigned long ultimoPrintCor = 0;

// ======================================
// MPU6050
// ======================================
#define MPU_ADDR 0x68

const int SINAL_GIRO = 1;

float offsetGz = 0;
float anguloAtual = 0;
float taxaGiro = 0;
float rumoAlvo = 0;
unsigned long tempoAnteriorGyro = 0;

float Kp = 7.5;
float Kd = 0.6;
const float CORRECAO_MAX = 45;

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

const int VELOCIDADE = 85;

// ======================================
// ULTRASSÔNICO TRASEIRO (liga / desliga)
// ======================================
#define TRIG 42
#define ECHO 40
const int DIST_ACIONAR = 5;      // cm
const int DIST_SOLTAR = 10;      // cm: a mão precisa sair antes do próximo toque
unsigned long ultimoUltra = 0;
bool maoPresente = false;
unsigned long ultimoAcionamento = 0;

bool andando = false;

// ======================================
// FITA LED
// ======================================
#define NUM_LEDS 300
#define DATA_PIN 39
CRGB leds[NUM_LEDS];

int velocidadeLed = 10;          // ms por LED na transição
CRGB corAlvo = CRGB(255, 0, 200);
int indiceLED = 0;
bool transicaoAtiva = false;
unsigned long inicioTransicao = 0;
unsigned long ultimoShow = 0;

// ======================================
// TABELA DE CORES (proporção R,G,B + brilho R+G+B)
// ======================================
struct CorRef {
  int id;
  const char* nome;
  float r, g, b;
  float brilho;
  CRGB led;
};

CorRef tabelaCores[] = {
  { 1, "BRANCO",   0.441, 0.374, 0.185, 2837, CRGB(255, 255, 255) },
  { 2, "AMARELO",  0.533, 0.335, 0.132, 1135, CRGB(255, 120, 0) },
  { 3, "ROSA",     0.688, 0.165, 0.147,  560, CRGB(255, 0, 50) },
  { 4, "TURQUESA", 0.296, 0.487, 0.217,  473, CRGB(0, 255, 50) },
  { 5, "VERMELHO", 0.711, 0.166, 0.123,  528, CRGB(255, 0, 0) },
  { 6, "LARANJA",  0.643, 0.232, 0.125,  560, CRGB(255, 60, 0) },
  { 7, "VERDE",    0.349, 0.429, 0.222,  315, CRGB(0, 255, 0) },
  { 8, "AZUL",     0.299, 0.325, 0.377,  385, CRGB(0, 0, 255) },
  // { 0, "CHAO",  0.000, 0.000, 0.000,    0, CRGB(0, 0, 0) },
};
const int NUM_CORES = sizeof(tabelaCores) / sizeof(tabelaCores[0]);

const float LIMITE_COR = 0.10;
const float PESO_BRILHO = 0.15;
const int LEITURAS_CONFIRMAR = 2;

int corCandidata = -1;
int contagemCandidata = 0;
int ultimaCorMostrada = -1;

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
  if (Wire.available() < 2) return (int16_t)offsetGz;
  int16_t gz = Wire.read() << 8;
  gz |= Wire.read();
  return gz;
}

void calibrarGyro() {
  long soma = 0;
  Serial.println("CALIBRANDO MPU6050... MANTENHA O RANDALL PARADO!");
  for (int i = 0; i < 1000; i++) {
    soma += lerGz();
    delay(2);
  }
  offsetGz = soma / 1000.0;
  Serial.println("CALIBRACAO CONCLUIDA!");
  anguloAtual = 0;
  taxaGiro = 0;
  tempoAnteriorGyro = micros();
}

void atualizarAngulo() {
  int16_t gz = lerGz();
  unsigned long agora = micros();
  float dt = (agora - tempoAnteriorGyro) / 1000000.0;
  tempoAnteriorGyro = agora;
  if (dt > 0.1) dt = 0.1;

  float taxa = SINAL_GIRO * (gz - offsetGz) / 131.0;
  if (fabs(taxa) < 0.3) taxa = 0;

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

void acionar(int esq, int dir) {
  acionarMotor(INF1, INF2, ENAF, esq);
  acionarMotor(INT1, INT2, ENAT, esq);
  acionarMotor(INF3, INF4, ENBF, dir);
  acionarMotor(INT3, INT4, ENBT, dir);
}

void pararMotores() {
  acionar(0, 0);
}

void andarReto(int vel) {
  float erro = anguloAtual - rumoAlvo;   // >0 = desviou p/ esquerda
  float c = Kp * erro + Kd * taxaGiro;
  c = constrain(c, -CORRECAO_MAX, CORRECAO_MAX);
  acionar(vel + c, vel - c);
}

// ======================================
// ULTRASSÔNICO
// ======================================
int lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duracao = pulseIn(ECHO, HIGH, 25000);
  return duracao * 0.034 / 2;
}

// Retorna true UMA vez cada vez que a mão chega perto
bool verificarAcionamento() {
  if (millis() - ultimoUltra < 80) return false;
  ultimoUltra = millis();

  int d = lerDistancia();

  if (!maoPresente && d > 0 && d < DIST_ACIONAR) {
    maoPresente = true;
    if (millis() - ultimoAcionamento > 1000) {  // evita acionar duas vezes seguidas
      ultimoAcionamento = millis();
      return true;
    }
  } else if (maoPresente && (d == 0 || d > DIST_SOLTAR)) {
    maoPresente = false;  // mão saiu: libera o próximo toque
  }
  return false;
}

// ======================================
// SENSOR DE COR
// ======================================
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

// Retorna o índice da cor confirmada (uma vez por passagem) ou -1
int verificarCor() {
  if (millis() - ultimaLeitura < intervaloSensor) return -1;
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

  if (idx >= 0 && contagemCandidata == LEITURAS_CONFIRMAR) return idx;
  return -1;
}

void calibrarCor() {
  pararMotores();
  andando = false;
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
// FITA LED
// ======================================
void iniciarTransicao(CRGB cor) {
  corAlvo = cor;
  indiceLED = 0;
  inicioTransicao = millis();
  transicaoAtiva = true;
}

void atualizarLeds() {
  if (!transicaoAtiva) return;
  if (millis() - ultimoShow < 40) return;  // show() de 300 LEDs leva ~9 ms
  ultimoShow = millis();

  int alvo = (millis() - inicioTransicao) / velocidadeLed + 1;
  if (alvo > NUM_LEDS) alvo = NUM_LEDS;
  while (indiceLED < alvo) leds[indiceLED++] = corAlvo;
  FastLED.show();

  if (indiceLED >= NUM_LEDS) transicaoAtiva = false;
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

  escreverMPU(0x6B, 0x00);
  escreverMPU(0x1A, 0x03);
  escreverMPU(0x1B, 0x00);
  delay(100);

  if (!tcs.begin()) {
    Serial.println("ERRO: TCS34725 NAO ENCONTRADO!");
    while (1);
  }

  FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 200));  // começa roxo
  FastLED.show();

  calibrarGyro();

  Serial.println("PRONTO! COLOQUE A MAO ATRAS DO RANDALL PARA INICIAR.");
}

// ======================================
// LOOP
// ======================================
void loop() {
  atualizarAngulo();

  if (Serial.available()) {
    char comando = Serial.read();
    if (comando == 'c') calibrarCor();
  }

  // ---------- liga / desliga pela mão ----------
  if (verificarAcionamento()) {
    andando = !andando;
    if (andando) {
      rumoAlvo = anguloAtual;   // segue reto na direção em que está agora
      corCandidata = -1;
      contagemCandidata = 0;
      Serial.println(">>> ANDANDO");
    } else {
      pararMotores();
      Serial.println(">>> PARADO");
    }
  }

  // ---------- anda e lê as cores ----------
  if (andando) {
    andarReto(VELOCIDADE);

    int idx = verificarCor();
    if (idx >= 0 && idx != ultimaCorMostrada) {
      ultimaCorMostrada = idx;
      Serial.print("COR LIDA: ");
      Serial.println(tabelaCores[idx].nome);
      iniciarTransicao(tabelaCores[idx].led);
    }
  }

  atualizarLeds();
}