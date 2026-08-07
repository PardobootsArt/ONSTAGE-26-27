<<<<<<< HEAD
const int s0 = 40;
const int s1 = 42;
const int s2 = 44;
const int s3 = 46;
const int out = 41;
=======
//giroscopio
#include <Wire.h>
#include <SoftwareSerial.h>
>>>>>>> c49b0716a391ee909287f4940042102daa6da7e5

//Pinos do led RGB
int Pin_RED = 7;
int Pin_GREEN = 8;
int Pin_BLUE = 9;

//Variaveis cores
int red = 0;
int green = 0;
int blue = 0;
bool camuflagemRandall = false;

<<<<<<< HEAD
// motores
int motorE1 = 22;
int motorE2 = 24;
int motorD1 = 26;
int motorD2 = 28;
int enbE = 11;
int enbD = 10;

// Define os pinos Trig e Echo do sensor ultrassônico
const int trigPin = 3;
const int echoPin = 4;
=======
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
>>>>>>> c49b0716a391ee909287f4940042102daa6da7e5

const int trigPin2 = 5;
const int echoPin2 = 6;
// Variáveis para a duração e a distância
long durationF;
int distFrente;
long durationT;
int distTras;

<<<<<<< HEAD
float Kp = 5.0;

int velocidadeBase = 60;

int distBase = 10;

void setup() {
  // put your setup code here, to run once:
  pinMode (motorE1, OUTPUT);
  pinMode (motorE2, OUTPUT);
  pinMode (motorD1, OUTPUT);
  pinMode (motorD2, OUTPUT);
  pinMode (enbE, OUTPUT);
  pinMode (enbD, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
=======
#define TRIG 5
#define ECHO 4
int posicaoAtual = 25;
bool sentidoHorario = true;
int contagemObjeto;

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
    int posTronco=posicaoAtual*10;
    girarTronco (90, posTronco);
    meuServo.write(posicaoAtual);
    delay(15);

    int distancia = lerDistancia();

    if (distancia > 0 && distancia < 30) {

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
//===============================================SETUP=========================================
void setup() {
  //COMUNICAÇÃO
>>>>>>> c49b0716a391ee909287f4940042102daa6da7e5
  Serial.begin(9600);
  //Led e sensor de cor
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);
  pinMode(Pin_RED, OUTPUT);
  pinMode(Pin_GREEN, OUTPUT);
  pinMode(Pin_BLUE, OUTPUT);
  Serial.begin(9600);
  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);

<<<<<<< HEAD
  digitalWrite (motorE1, HIGH);
  digitalWrite (motorE2, LOW);
  digitalWrite (motorD1, HIGH);
  digitalWrite (motorD2, LOW);

}

void loop() {

  //Detecta a cor
  color();

  if (camuflagemRandall == false) {
    Serial.print("Vermelho :");
    Serial.print(red, DEC);
    Serial.print(" Verde : ");
    Serial.print(green, DEC);
    Serial.print(" Azul : ");
    Serial.print(blue, DEC);
    Serial.println();
    Serial.println("Procurando Verde");
    AcenderLed(175, 0, 255);

    //motores
    distFrente = calDistanciaF ();
    distTras = calDistanciaT ();


   int erro = (distFrente - distTras) * (-1);   
   int correcao = Kp * erro;

    int velEsq = velocidadeBase + correcao;
    int velDir = velocidadeBase - correcao;

    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    analogWrite(enbE, velEsq);
    analogWrite(enbD, velDir);

    Serial.print("Erro: ");
    Serial.print(erro);

   Serial.print("  Esq: ");
   Serial.print(velEsq);

   Serial.print("  Dir: ");
    Serial.println(velDir);

    delay(10);
    if (green > 180 && green < 600 && red > 300 && green < blue) {
      camuflagemRandall = true;
      Serial.println("Iniciando Camuflagem Randall...");
      delay(50);
      digitalWrite (motorE1, LOW);
      digitalWrite (motorE2, LOW);
      digitalWrite (motorD1, LOW);
      digitalWrite (motorD2, LOW);
      delay (500);
      analogWrite (enbE, 200);
      analogWrite (enbD, 200);
      digitalWrite (motorE1, HIGH);
      digitalWrite (motorE2, LOW);
      digitalWrite (motorD1, LOW);
      digitalWrite (motorD2, HIGH);

      delay (1000);
      
      digitalWrite (motorE1, HIGH);
      digitalWrite (motorE2, LOW);
      digitalWrite (motorD1, HIGH);
      digitalWrite (motorD2, LOW);
    }
  }

  if (camuflagemRandall == true) {

    Serial.print("Vermelho :");
    Serial.print(red, DEC);
    Serial.print(" Verde : ");
    Serial.print(green, DEC);
    Serial.print(" Azul : ");
    Serial.print(blue, DEC);
    Serial.println();

    //Verifica se a cor branca foi detectada
    if (red < 100 && green < 100 && blue < 100) {
      Serial.println("BRANCO");
      AcenderLed(175, 0, 255);
    }
    //Verifica se a cor amarela foi detectada
    else if (red < 60 && green < 100 && green < blue) {
      Serial.println("AMARELO");
      AcenderLed(255, 110, 0);
    }
    //Verifica se a cor vermelha foi detectada
    else if (red < blue && blue < green && red < 100) {
      Serial.println("VERMELHO");
      AcenderLed(255, 0, 0);
    }

    //Verifica se a cor azul foi detectada
    else if (blue < red && blue < green && blue < 1000) {
      Serial.println("AZUL");
      AcenderLed(0, 0, 255);
    }

  }
  Serial.println();

  //Delay para apagar os leds e reiniciar o processo
  delay(300);

  
}


void color() {
  //Rotina que le o valor das cores
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  //count OUT, pRed, RED
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);
  //count OUT, pBLUE, BLUE
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);
  //count OUT, pGreen, GREEN
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
}

void AcenderLed(int R, int G, int B) {
  analogWrite(Pin_RED, R);
  analogWrite(Pin_GREEN, G);
  analogWrite(Pin_BLUE, B);
}
  


int calDistanciaF() {

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Define o trigPin no estado HIGH por 10 micro segundos
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    durationF = pulseIn(echoPin, HIGH);  // Lê o echoPin, retorna o tempo de viagem da onda sonora em microssegundos
    distFrente = durationF * 0.034 / 2;
    return distFrente;
  }

  int calDistanciaT() {

    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    // Define o trigPin no estado HIGH por 10 micro segundos
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);
    durationT = pulseIn(echoPin2, HIGH);  // Lê o echoPin, retorna o tempo de viagem da onda sonora em microssegundos
    distTras = durationT * 0.034 / 2;
    return distTras;
  }



















// 
=======
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
  meuServo.write(25);
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
  procurarObjeto();
  Serial.print("graus: ");
  Serial.println(degrees);
  delay(500);
}
>>>>>>> c49b0716a391ee909287f4940042102daa6da7e5
