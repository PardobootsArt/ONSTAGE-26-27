
#include <Wire.h>
#include "Adafruit_TCS34725.h"
//#include <FastLED.h>

// ======================================
// SENSOR TCS34725
// ======================================

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_1X);

// ======================================
// FITA WS2811
// ======================================

#define NUM_LEDS 300
#define DATA_PIN 39

//CRGB leds[NUM_LEDS];

int velocidade = 40;
//CRGB corAlvo = CRGB(255, 0, 200);

int indiceLED = NUM_LEDS - 1;
bool transicaoAtiva = false;

// Controle de tempo
unsigned long tempoAnterior = 0;
unsigned long ultimaLeitura = 0;

unsigned long inicioIdentificacao = 0;
bool primeiraAtualizacao = false;
// ======================================
// MOTORES - L298N
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

const int velocidadeMotor = 100;

const int MPU = 0x68;

float anguloZ = 0.0;
unsigned long ultimoTempo;

float offsetGz = 0;

float ultimoErro = 0;

float Kp = 4.0;
float Kd = 1.5;
float ang;

// ======================================
// CONTROLE DAS CORES
// ======================================

// Cores:
// 1 = Amarelo
// 2 = Rosa
// 3 = Turquesa
// 4 = Vermelho
// 5 = Laranja
// 6 = Verde
// 7 = Azul

bool coresEncontradas[8] = { false };

int quantidadeCores = 0;

bool finalizado = false;

const unsigned long intervaloSensor = 100;

// ======================================
// CONTROLE DOS MOTORES
// ======================================

void andarFrente() {

  digitalWrite(INF1, HIGH);
  digitalWrite(INF2, LOW);
  digitalWrite(INF3, HIGH);
  digitalWrite(INF4, LOW);

  digitalWrite(INT1, HIGH);
  digitalWrite(INT2, LOW);
  digitalWrite(INT3, HIGH);
  digitalWrite(INT4, LOW);
}

void pararMotores() {

  analogWrite(ENAF, 0);
  analogWrite(ENBF, 0);

  analogWrite(ENAT, 0);
  analogWrite(ENBT, 0);

  digitalWrite(INF1, LOW);
  digitalWrite(INF2, LOW);
  digitalWrite(INF3, LOW);
  digitalWrite(INF4, LOW);

  digitalWrite(INT1, LOW);
  digitalWrite(INT2, LOW);
  digitalWrite(INT3, LOW);
  digitalWrite(INT4, LOW);
}

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

// ======================================
// SETUP
// ======================================

void setup() {

  Serial.begin(115200);

  pinMode(ENAF, OUTPUT);
  pinMode(ENBF, OUTPUT);
  pinMode(ENAT, OUTPUT);
  pinMode(ENBT, OUTPUT);

  pinMode(INT1, OUTPUT);
  pinMode(INT2, OUTPUT);
  pinMode(INT3, OUTPUT);
  pinMode(INT4, OUTPUT);

  pinMode(INF1, OUTPUT);
  pinMode(INF2, OUTPUT);
  pinMode(INF3, OUTPUT);
  pinMode(INF4, OUTPUT);

  pararMotores();

  iniciarMPU();
  Serial.println("MPU pronto!");

  inicioIdentificacao = millis();
  primeiraAtualizacao = true;

  Serial.println("COR DETECTADA!");

  if (!tcs.begin()) {
    Serial.println("ERRO: TCS34725 nao encontrado!");
    while (1)
      ;
  }

  //FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
  //FastLED.setBrightness(100);

  //fill_solid(leds, NUM_LEDS, corAlvo);
  //FastLED.show();

  Serial.println("RANDALL INICIADO!");
}

// ======================================
// LOOP
// ======================================

void loop() {

  if (finalizado) {
    pararMotores();
    return;
  } else {
    andarFrente();

    float erro = obterAngulo();

    float correcaoP = Kp * erro;
    float correcaoD = Kd * (ultimoErro - erro);
    float correcaoFinal = correcaoP + correcaoD;
    float velEsq = velocidadeMotor + correcaoFinal;
    float velDir = velocidadeMotor - correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    analogWrite(ENAT, velEsq);
    analogWrite(ENBT, velDir);
    analogWrite(ENAF, velEsq);
    analogWrite(ENBF, velDir);

    ultimoErro = erro;
    /*
    if (!transicaoAtiva && millis() - ultimaLeitura >= intervaloSensor) {

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

      // AMARELO
      if (r > 900 && r < 1100 && g > 700 && g < 800 && b > 200 && b < 300) {

        novaCor = 1;
      }

      // ROSA
      else if (r > 800 && r < 900 && g > 300 && g < 400 && b > 300 && b < 400) {

        novaCor = 2;
      }

      // TURQUESA
      else if (r > 100 && r < 230 && g > 300 && g < 350 && b > 230 && b < 280) {

        novaCor = 3;
      }

      // VERMELHO
      else if (r > 550 && r < 650 && g > 130 && g < 170 && b > 110 && b < 130) {

        novaCor = 4;
      }

      // LARANJA
      else if (r > 810 && r < 860 && g > 240 && g < 280 && b > 150 && b < 200) {

        novaCor = 5;
      }

      // VERDE
      else if (r > 150 && r < 170 && g > 250 && g < 290 && b > 110 && b < 180) {

        novaCor = 6;
      }

      // AZUL
      else if (r > 120 && r < 160 && g > 175 && g < 200 && b > 185 && b < 220) {

        novaCor = 7;
      }

      // ==================================
      // 2. IDENTIFICAÇÃO DE COR NOVA
      // ==================================

      if (novaCor != 0 && !coresEncontradas[novaCor]) {

        pararMotores();

        Serial.println("1 - COR IDENTIFICADA");

        inicioIdentificacao = millis();
        primeiraAtualizacao = true;

        coresEncontradas[novaCor] = true;
        quantidadeCores++;

        Serial.print("NOVA COR IDENTIFICADA! Total: ");
        Serial.println(quantidadeCores);

        if (novaCor == 1) {
          corAlvo = CRGB(255, 120, 0);
          Serial.println("AMARELO");
        }

        else if (novaCor == 2) {
          corAlvo = CRGB(255, 0, 50);
          Serial.println("ROSA");
        }

        else if (novaCor == 3) {
          corAlvo = CRGB(60, 240, 150);
          Serial.println("TURQUESA");
        }

        else if (novaCor == 4) {
          corAlvo = CRGB(255, 0, 0);
          Serial.println("VERMELHO");
        }

        else if (novaCor == 5) {
          corAlvo = CRGB(255, 60, 0);
          Serial.println("LARANJA");
        }

        else if (novaCor == 6) {
          corAlvo = CRGB(0, 255, 0);
          Serial.println("VERDE");
        }

        else if (novaCor == 7) {
          corAlvo = CRGB(0, 0, 255);
          Serial.println("AZUL");
        }

        Serial.println("2 - ENVIANDO PRIMEIRO LED");

        // Acende imediatamente o primeiro LED
        leds[0] = corAlvo;
        FastLED.show();

        // Prepara a transição do último LED até o primeiro
        indiceLED = NUM_LEDS - 1;
        transicaoAtiva = true;
        tempoAnterior = millis() - velocidade;
      }
    }

    // ====================================
    // 3. TRANSIÇÃO DOS LEDS
    // ====================================

    if (transicaoAtiva && millis() - tempoAnterior >= velocidade) {

      tempoAnterior = millis();

      leds[indiceLED] = corAlvo;

      if (primeiraAtualizacao) {
        Serial.print("TEMPO ATE O PRIMEIRO LED: ");
        Serial.print(millis() - inicioIdentificacao);
        Serial.println(" ms");

        primeiraAtualizacao = false;
      }

      FastLED.show();

      indiceLED--;

      if (indiceLED < 0) {

        transicaoAtiva = false;

        Serial.println("TRANSICAO CONCLUIDA!");

        // =================================
        // 4. VERIFICA SE CHEGOU A 7 CORES
        // =================================

        if (quantidadeCores >= 7) {

          pararMotores();

          finalizado = true;

          Serial.println("7 CORES DIFERENTES IDENTIFICADAS!");
          Serial.println("RANDALL FINALIZADO!");

        } else {

          andarFrente();

          Serial.println("RANDALL VOLTOU A ANDAR!");
        }
      }
    }
    */
  }
}
