#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Criação do objeto para o sensor TCS34725
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_300MS,
  TCS34725_GAIN_1X
);

#include "mpu6050.h"

//===== PINOS =====

// LED RGB
int Pin_RED = 7;
int Pin_GREEN = 8;
int Pin_BLUE = 9;

// Variáveis cores
int red = 0;
int green = 0;
int blue = 0;

bool camuflagemRandall = false;

// Motores
int motorE1 = 22;
int motorE2 = 24;

int motorD1 = 26;
int motorD2 = 28;

int enbE = 11;
int enbD = 10;

// Sensores ultrassônicos
const int trigPin = 3;
const int echoPin = 4;

const int trigPin2 = 5;
const int echoPin2 = 6;

// Variáveis para duração e distância
long durationF;
int distFrente;

long durationT;
int distTras;

//===== MPU6050 =====

int ultimoErro = 0;

int Kp = 15;
int Kd = 20;

int velocidadeBase = 75;

//===== CORES =====

int corAtual = 0;
int ultimaCor = 0;

int quantidadeCores = 0;

int corEmConfirmacao = 0;
int contadorLeituras = 0;

bool encontrouTresCores = false;

int a;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  // Inicializa o sensor TCS34725
  if (!tcs.begin()) {

    Serial.println(
      "Não foi possível encontrar o TCS34725. "
      "Verifique as conexões!"
    );

    while (1);
  }

  Serial.println("Sensor TCS34725 inicializado.\n");

  // MPU6050
  Serial2.begin(115200);

  mpu_begin();

  Serial.println("Calibrando, deixa parado!");

  delay(1000);

  mpu_calibrate(200);

  Serial.println("===== Calibrado! =====\n");

  mpu_reset();

  // Motores
  pinMode(motorE1, OUTPUT);
  pinMode(motorE2, OUTPUT);

  pinMode(motorD1, OUTPUT);
  pinMode(motorD2, OUTPUT);

  pinMode(enbE, OUTPUT);
  pinMode(enbD, OUTPUT);

  // Ultrassônicos
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // LED RGB
  pinMode(Pin_RED, OUTPUT);
  pinMode(Pin_GREEN, OUTPUT);
  pinMode(Pin_BLUE, OUTPUT);

  AcenderLed(175, 0, 255);

  // Espera objeto ficar a menos de 5 cm
  a = 0;

  while (a < 1) {

    int distancia = calDistanciaF();

    if (distancia > 0 && distancia < 5) {
      a = 3;
    }
  }
}


// =====================================================
// DIREÇÃO DOS MOTORES
// =====================================================

void direcaoMotores(int direcao) {

  // PARADO
  if (direcao == -1) {

    digitalWrite(motorE1, LOW);
    digitalWrite(motorE2, LOW);

    digitalWrite(motorD1, LOW);
    digitalWrite(motorD2, LOW);
  }

  // FRENTE
  if (direcao == 1) {

    digitalWrite(motorE1, HIGH);
    digitalWrite(motorE2, LOW);

    digitalWrite(motorD1, HIGH);
    digitalWrite(motorD2, LOW);
  }

  // GIRO
  if (direcao == 2) {

    digitalWrite(motorE1, HIGH);
    digitalWrite(motorE2, LOW);

    digitalWrite(motorD1, LOW);
    digitalWrite(motorD2, HIGH);
  }
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  mpu_loop();

  // ===================================================
  // JÁ ENCONTROU 3 CORES
  // ===================================================

  if (encontrouTresCores) {

    analogWrite(enbE, 0);
    analogWrite(enbD, 0);

    return;
  }


  // ===================================================
  // PRIMEIRA PARTE
  // ANDANDO ATÉ ENCONTRAR O PRETO
  // ===================================================

  if (camuflagemRandall == false) {

    direcaoMotores(1);


    // =================================================
    // LEITURA DO SENSOR DE COR
    // =================================================

    uint16_t r, g, b, c;

    tcs.getRawData(&r, &g, &b, &c);

    red = r;
    green = g;
    blue = b;


    // =================================================
    // CORREÇÃO MPU6050
    // NÃO MEXER
    // =================================================

    int erro = getAngleZ();

    int correcaoP = Kp * erro;

    int correcaoD = Kd * (ultimoErro - erro);

    int correcaoFinal = correcaoP + correcaoD;

    int velEsq = velocidadeBase + correcaoFinal;

    int velDir = velocidadeBase - correcaoFinal;

    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    analogWrite(enbE, velEsq);
    analogWrite(enbD, velDir);

    ultimoErro = erro;


    // =================================================
    // SERIAL
    // =================================================

    Serial.print("VelE: ");
    Serial.print(velEsq);

    Serial.print(" | VelD: ");
    Serial.println(velDir);


    // =================================================
    // DETECÇÃO DO PRETO
    // =================================================

    if (green > 500 && red > 500 && blue > 400) {

      Serial.println("PRETO DETECTADO");

      camuflagemRandall = true;

      AcenderLed(175, 0, 255);

      analogWrite(enbE, 65);
      analogWrite(enbD, 75);

      direcaoMotores(1);

      delay(1550);

      direcaoMotores(-1);

      delay(1000);

      analogWrite(enbE, 165);
      analogWrite(enbD, 210);

      digitalWrite(motorE1, HIGH);
      digitalWrite(motorE2, LOW);

      digitalWrite(motorD1, LOW);
      digitalWrite(motorD2, HIGH);

      delay(950);

      direcaoMotores(-1);

      delay(2000);
    }
  }


  // ===================================================
  // SEGUNDA PARTE
  // DETECÇÃO DAS CORES
  // ===================================================

  if (camuflagemRandall == true) {

    // =================================================
    // MOTORES
    // =================================================

    direcaoMotores(1);


    // =================================================
    // ULTRASSÔNICOS
    // =================================================

    distFrente = calDistanciaF() + 2;

    distTras = calDistanciaT();


    // =================================================
    // CORREÇÃO DOS MOTORES
    // =================================================

    int erro = distFrente - distTras;

    int correcaoP = Kp * erro;

    int correcaoD = Kd * (ultimoErro - erro);

    int correcaoFinal = correcaoP + correcaoD;

    int velEsq = velocidadeBase + correcaoFinal;

    int velDir = velocidadeBase - correcaoFinal;

    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    analogWrite(enbE, velEsq);
    analogWrite(enbD, velDir + 5);

    ultimoErro = erro;


    // =================================================
    // SENSOR DE COR
    // =================================================

    uint16_t r, g, b, c;

    tcs.getRawData(&r, &g, &b, &c);

    red = r;
    green = g;
    blue = b;


    // =================================================
    // SERIAL - CORES NA HORIZONTAL
    // =================================================

    Serial.print("Azul: ");
    Serial.print(blue);

    Serial.print(" | Vermelho: ");
    Serial.print(red);

    Serial.print(" | Verde: ");
    Serial.println(green);


    Serial.print("Quantidade de Cores: ");
    Serial.println(quantidadeCores);


    Serial.print("Ultra Frente: ");
    Serial.print(distFrente);

    Serial.print(" | Ultra Tras: ");
    Serial.println(distTras);


    // =================================================
    // CLASSIFICAÇÃO DAS CORES
    // =================================================

  
    if (red < 90 && green < 90 && blue < 90) {

      corAtual = 1;

      Serial.println("BRANCO");

      AcenderLed(150, 0, 255);
    }

    else if (r > 2200 &&  r < 6500 &&  g > 1500 &&  g < 5200 &&  b > 500 && b < 2000) {

      corAtual = 2;

      Serial.println("AMARELO");

      AcenderLed(255, 130, 0);
    }

    else if (r > 2000 && r < 4000 &&  g > 600 && g < 800 && b > 500 && b < 700 ) {

      corAtual = 3;

      Serial.println("ROSA");

      AcenderLed(255, 0, 175);
    }

    else if (r > 600 && r < 1200 && g > 1000 && g < 2200 && b > 800 && b < 1500 ) {

      corAtual = 4;

      Serial.println("AZUL");

      AcenderLed(64, 224, 208);
    }


    // NENHUMA COR
    else {

      corAtual = 0;
    }


    // =================================================
    // CONFIRMAÇÃO DA COR
    // =================================================

    if (corAtual != 0 && corAtual != 1) {

      if (corAtual != corEmConfirmacao) {

        corEmConfirmacao = corAtual;

        contadorLeituras = 1;

      } else {

        contadorLeituras++;
      }


      // Confirma após 10 leituras
      if (contadorLeituras >= 10) {

        // Só conta se for diferente da última cor
        if (corEmConfirmacao != ultimaCor) {

          quantidadeCores++;

          ultimaCor = corEmConfirmacao;

          Serial.print(
            "NOVA FILEIRA CONFIRMADA! Quantidade: "
          );

          Serial.println(quantidadeCores);
        }

        contadorLeituras = 0;
      }
    }


    // =================================================
    // ENCONTROU 3 CORES
    // =================================================

    if (quantidadeCores >= 3) {

      encontrouTresCores = true;

      analogWrite(enbE, 65);
      analogWrite(enbD, 65);

      delay(1000);

      analogWrite(enbE, 0);
      analogWrite(enbD, 0);

      Serial.println(
        "3 CORES DETECTADAS - PARANDO!"
      );
    }
  }
}


// =====================================================
// LED RGB
// =====================================================

void AcenderLed(int R, int G, int B) {

  analogWrite(Pin_RED, R);
  analogWrite(Pin_GREEN, G);
  analogWrite(Pin_BLUE, B);
}


// =====================================================
// ULTRASSÔNICO DA FRENTE
// =====================================================

int calDistanciaF() {

  digitalWrite(trigPin, LOW);

  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  durationF = pulseIn(echoPin, HIGH);

  distFrente = durationF * 0.034 / 2;

  delay(30);

  return distFrente;
}


// =====================================================
// ULTRASSÔNICO DE TRÁS
// =====================================================

int calDistanciaT() {

  digitalWrite(trigPin2, LOW);

  delayMicroseconds(2);

  digitalWrite(trigPin2, HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPin2, LOW);

  durationT = pulseIn(echoPin2, HIGH);

  distTras = durationT * 0.034 / 2;

  delay(30);

  return distTras;
}

