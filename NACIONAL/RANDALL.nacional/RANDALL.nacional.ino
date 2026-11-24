
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

// ======================================
// MPU6050
// ======================================

#define MPU_ADDR 0x68

float offsetGz = 0;
float anguloAtual = 0;

unsigned long tempoAnteriorGyro = 0;

// ======================================
// FITA WS2811
// ======================================

#define NUM_LEDS 300
#define DATA_PIN 39

CRGB leds[NUM_LEDS];

int velocidade = 10;

CRGB corAlvo = CRGB(255, 0, 200);

// ======================================
// MOTORES
// ======================================

#define ENAT 11
#define ENBT 10

#define INT1 22
#define INT2 24
#define INT3 26
#define INT4 28

#define INF1 30
#define INF2 32
#define INF3 34
#define INF4 36

#define ENAF 9
#define ENBF 8

// VELOCIDADES

const int velocidadeAntesCurva = 80;
const int velocidadeDepoisCurva = 70;
const int velocidadeCurva = 200;

// ======================================
// CONTROLE DA CURVA
// ======================================

const float anguloCurva = 90.0;

const unsigned long tempoAntesDaCurva = 2500;

// ======================================
// CONTROLE DAS ETAPAS
// ======================================

bool procurandoBranco = true;

// ======================================
// CONTROLE DOS LEDS
// ======================================

int indiceLED = 0;

bool transicaoAtiva = false;

unsigned long tempoAnterior = 0;
unsigned long ultimaLeitura = 0;

const unsigned long intervaloSensor = 100;

// ======================================
// CONTROLE DA MISSÃO
// ======================================

// LARANJA -> AZUL -> VERMELHO ->
// TURQUESA -> AMARELO -> ROSA -> VERDE

const int ordemCores[7] = {
  6, 8, 5, 4, 2, 3, 7
};

int etapaAtual = 0;

bool missaoFinalizada = false;
bool aguardandoParada = false;

unsigned long tempoFinalizacao = 0;

const unsigned long tempoExtra = 2000;

// ======================================
// MPU6050 - ESCRITA
// ======================================

void escreverMPU(byte registrador, byte valor) {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(registrador);
  Wire.write(valor);
  Wire.endTransmission();
}

// ======================================
// MPU6050 - LEITURA DO EIXO Z
// ======================================

int16_t lerGz() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47);
  Wire.endTransmission(false);

  Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)2, (uint8_t) true);

  if (Wire.available() < 2) {
    return 0;
  }

  int16_t gz = Wire.read() << 8;
  gz |= Wire.read();

  return gz;
}

// ======================================
// CALIBRAÇÃO DO GIROSCÓPIO
// ======================================

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

  tempoAnteriorGyro = micros();
}

// ======================================
// ATUALIZAÇÃO DO ÂNGULO
// ======================================

void atualizarAngulo() {

  int16_t gz = lerGz();

  unsigned long agora = micros();

  float dt = (agora - tempoAnteriorGyro) / 1000000.0;

  tempoAnteriorGyro = agora;

  float velocidadeAngular = (gz - offsetGz) / 131.0;

  anguloAtual += velocidadeAngular * dt;
}

// ======================================
// NORMALIZAÇÃO DO ÂNGULO
// ======================================

float normalizarAngulo(float angulo) {

  while (angulo > 180) {
    angulo -= 360;
  }

  while (angulo < -180) {
    angulo += 360;
  }

  return angulo;
}

// ======================================
// MOVIMENTO PARA FRENTE
// ======================================

void andarFrente(int velocidadeMotor) {

  digitalWrite(INF1, HIGH);
  digitalWrite(INF2, LOW);

  digitalWrite(INF3, HIGH);
  digitalWrite(INF4, LOW);

  digitalWrite(INT1, HIGH);
  digitalWrite(INT2, LOW);

  digitalWrite(INT3, HIGH);
  digitalWrite(INT4, LOW);

  analogWrite(ENAT, velocidadeMotor);
  analogWrite(ENBT, velocidadeMotor);
  analogWrite(ENAF, velocidadeMotor);
  analogWrite(ENBF, velocidadeMotor);
}

// ======================================
// PARAR MOTORES
// ======================================

void pararMotores() {

  analogWrite(ENAT, 0);
  analogWrite(ENBT, 0);
  analogWrite(ENAF, 0);
  analogWrite(ENBF, 0);
}

// ======================================
// CURVA DE 90 GRAUS PARA A ESQUERDA
// ======================================


void girarEsquerda90() {

  while (anguloAtual >= 85) {
    Serial.println(anguloAtual);

    // LADO ESQUERDO: PARA TRÁS

    digitalWrite(INF1, LOW);
    digitalWrite(INF2, HIGH);

    digitalWrite(INT1, HIGH);
    digitalWrite(INT2, LOW);

    // LADO DIREITO: PARA FRENTE

    digitalWrite(INF3, LOW);
    digitalWrite(INF4, HIGH);

    digitalWrite(INT3, HIGH);
    digitalWrite(INT4, LOW);

    // ACIONA OS QUATRO MOTORES

    analogWrite(ENAT, velocidadeCurva);
    analogWrite(ENBT, velocidadeCurva);
    analogWrite(ENAF, velocidadeCurva);
    analogWrite(ENBF, velocidadeCurva);
  }
  pararMotores();
}


// ======================================
// SETUP
// ======================================

void setup() {

  Serial.begin(115200);

  Wire.begin();

  // CONFIGURAÇÃO DOS MOTORES

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

  // INICIALIZAÇÃO DO MPU6050

  escreverMPU(0x6B, 0x00);
  escreverMPU(0x1B, 0x00);

  delay(100);

  calibrarGyro();

  // INICIALIZAÇÃO DO SENSOR

  if (!tcs.begin()) {

    Serial.println("ERRO: TCS34725 NAO ENCONTRADO!");

    while (1)
      ;
  }

  // ==================================
  // INICIALIZAÇÃO DA FITA LED
  // ==================================

  FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);

  FastLED.setBrightness(100);

  // TODOS OS LEDS COMEÇAM ROXOS

  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 200));

  FastLED.show();

  Serial.println("-------------------------");
  Serial.println("FITA LED INICIADA EM ROXO.");

  // COMEÇA PROCURANDO O BRANCO

  andarFrente(velocidadeAntesCurva);

  Serial.println("MISSAO INICIADA!");
  Serial.println("ETAPA 1: PROCURANDO BRANCO.");
  Serial.println("VELOCIDADE: 80");
  procurandoBranco = true;
}

// ======================================
// LOOP
// ======================================

void loop() {

  // ====================================
  // 1. AJUSTE DA VELOCIDADE DOS LEDS
  // ====================================

  if (Serial.available()) {

    char comando = Serial.read();

    if (comando == '1') velocidade = 10;
    else if (comando == '2') velocidade = 20;
    else if (comando == '3') velocidade = 30;
    else if (comando == '4') velocidade = 40;
    else if (comando == '5') velocidade = 60;
    else if (comando == '6') velocidade = 80;
    else if (comando == '7') velocidade = 100;

    if (comando >= '1' && comando <= '7') {

      Serial.print("VELOCIDADE DOS LEDS: ");
      Serial.print(velocidade);
      Serial.println(" ms por LED");
    }
  }

  // ====================================
  // 2. LEITURA DO SENSOR
  // ====================================

  if (!missaoFinalizada && !aguardandoParada && millis() - ultimaLeitura >= intervaloSensor) {

    ultimaLeitura = millis();

    uint16_t r, g, b, c;

    tcs.getRawData(&r, &g, &b, &c);

    Serial.print("R: ");
    Serial.print(r);
    Serial.print(" | G: ");
    Serial.print(g);
    Serial.print(" | B: ");
    Serial.println(b);

    int novaCor = 0;

    // ==================================
    // ETAPA 1: PROCURAR SOMENTE BRANCO
    // ==================================

    if (procurandoBranco) {

      if (r > 1000 && r < 1500 && g > 950 && g < 1175 && b > 400 && b < 650) {

        Serial.println("-------------------------");
        Serial.println("BRANCO IDENTIFICADO!");

        // A FITA CONTINUA ROXA.
        // O BRANCO NÃO ALTERA OS LEDS.

        // ANDA POR 500 MS COM PWM 80

        Serial.println("ANDANDO ANTES DA CURVA...");

        andarFrente(velocidadeAntesCurva);

        delay(tempoAntesDaCurva);

        pararMotores();

        // EXECUTA A CURVA

        girarEsquerda90();

        // PASSA PARA A SEGUNDA ETAPA

        procurandoBranco = false;

        Serial.println("-------------------------");
        Serial.println("CURVA FINALIZADA!");
        Serial.println("ETAPA 2: PROCURANDO AS 7 CORES.");
        Serial.println("VELOCIDADE: 70");

        andarFrente(velocidadeDepoisCurva);
      }
    }

    // ==================================
    // ETAPA 2: PROCURAR AS OUTRAS CORES
    // ==================================

    else {

      // AMARELO

      if (r > 510 && r < 700 && g > 330 && g < 430 && b > 120 && b < 180) {

        novaCor = 2;
      }

      // ROSA

      else if (r > 320 && r < 450 && g > 75 && g < 110 && b > 65 && b < 100) {

        novaCor = 3;
      }

      // TURQUESA

      else if (r > 110 && r < 170 && g > 180 && g < 280 && b > 75 && b < 130) {

        novaCor = 4;
      }

      // VERMELHO

      else if (r > 300 && r < 450 && g > 65 && g < 110 && b > 45 && b < 85) {

        novaCor = 5;
      }

      // LARANJA

      else if (r > 250 && r < 470 && g > 100 && g < 160 && b > 50 && b < 90) {

        novaCor = 6;
      }

      // VERDE

      else if (r > 80 && r < 140 && g > 90 && g < 180 && b > 55 && b < 85) {

        novaCor = 7;
      }

      // AZUL

      else if (r > 90 && r < 140 && g > 90 && g < 160 && b > 100 && b < 190) {

        novaCor = 8;
      }

      // ==================================
      // IDENTIFICAÇÃO NA ORDEM DA MISSÃO
      // ==================================

      if (novaCor != 0 && etapaAtual < 7 && novaCor == ordemCores[etapaAtual]) {

        Serial.println("-------------------------");
        Serial.println("NOVA COR IDENTIFICADA!");

        if (novaCor == 2) {

          corAlvo = CRGB(255, 120, 0);
          Serial.println("COR: AMARELO");
        }

        else if (novaCor == 3) {

          corAlvo = CRGB(255, 0, 50);
          Serial.println("COR: ROSA");
        }

        else if (novaCor == 4) {

          corAlvo = CRGB(0, 255, 50);
          Serial.println("COR: TURQUESA");
        }

        else if (novaCor == 5) {

          corAlvo = CRGB(255, 0, 0);
          Serial.println("COR: VERMELHO");
        }

        else if (novaCor == 6) {

          corAlvo = CRGB(255, 60, 0);
          Serial.println("COR: LARANJA");
        }

        else if (novaCor == 7) {

          corAlvo = CRGB(0, 255, 0);
          Serial.println("COR: VERDE");
        }

        else if (novaCor == 8) {

          corAlvo = CRGB(0, 0, 255);
          Serial.println("COR: AZUL");
        }

        indiceLED = 0;
        transicaoAtiva = true;

        etapaAtual++;

        Serial.print("PROGRESSO: ");
        Serial.print(etapaAtual);
        Serial.println("/7");

        // FINALIZAÇÃO DAS SETE CORES

        if (etapaAtual >= 7) {

          aguardandoParada = true;
          tempoFinalizacao = millis();

          Serial.println("-------------------------");
          Serial.println("7 CORES IDENTIFICADAS!");
          Serial.println("RANDALL CONTINUARA ANDANDO.");
        }
      }
    }
  }

  // ====================================
  // 3. TEMPO EXTRA APÓS AS SETE CORES
  // ====================================

  if (aguardandoParada && millis() - tempoFinalizacao >= tempoExtra) {

    pararMotores();

    aguardandoParada = false;
    missaoFinalizada = true;

    Serial.println("-------------------------");
    Serial.println("MISSAO CONCLUIDA!");
    Serial.println("MOTORES PARADOS.");
  }

  // ====================================
  // 4. TRANSIÇÃO DA FITA LED
  // ====================================

  if (transicaoAtiva && millis() - tempoAnterior >= velocidade) {

    tempoAnterior = millis();

    leds[indiceLED] = corAlvo;

    FastLED.show();

    indiceLED++;

    if (indiceLED >= NUM_LEDS) {

      transicaoAtiva = false;

      Serial.println("TRANSICAO CONCLUIDA!");
    }
  }
}
