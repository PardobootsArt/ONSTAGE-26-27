//giroscopio
#include <Wire.h>

const int MPU = 0x68;

float anguloZ = 0.0;
unsigned long ultimoTempo;

float offsetGz = 0;

//radar
#include <Servo.h>

Servo meuServo;

#define TRIG 8
#define ECHO 9

//correcao
float Kp = 10.0;
float Kd = 5.0;
int cronometro;

int lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH);

  return duracao * 0.034 / 2;
}

int procurarObjeto() {

  for(int posicao=15;posicao<=140;posicao++){  
    meuServo.write(posicao);
    delay(30);
    int distancia = lerDistancia();
  
    if (distancia > 0 && distancia < 20)
    {
    return posicao;
    }
  }

  for(int posicao=140;posicao>15;posicao--){  
    meuServo.write(posicao);
    delay(30);
    int distancia = lerDistancia();

    if (distancia > 0 && distancia < 20)
    {
    return posicao;
    }

  }
}


//função para o giroscopio
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
    Wire.write(0x47); // GyZ
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


// motores
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

void setup() {
  //cronometro
  cronometro = 0;

  //giroscopio
  Serial.begin(9600);

  iniciarMPU();

  Serial.println("MPU pronto!");

  //radar
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  meuServo.attach(servo);


  //motores
  pinMode (motorEF1, OUTPUT);
  pinMode (motorEF2, OUTPUT);
  pinMode (motorDF1, OUTPUT);
  pinMode (motorDF2, OUTPUT);
  pinMode (motorET1, OUTPUT);
  pinMode (motorET2, OUTPUT);
  pinMode (motorDT1, OUTPUT);
  pinMode (motorDT2, OUTPUT);

  pinMode (enbEF, OUTPUT);
  pinMode (enbDF, OUTPUT);
  pinMode (enbET, OUTPUT);
  pinMode (enbDT, OUTPUT);

  pinMode (20, INPUT);
  pinMode (21, INPUT);

}

void loop() {
  frente (100, 25);
  delay (1000);
  esquerda (100, 25);
  delay (1000);
  tras (100,25);
  delay (1000);
  direita (100, 25);
  delay (1000);
  
  //float angulo = obterAngulo();

  //Serial.print("Angulo: ");
  //Serial.println(angulo);

  //delay(10);

  //int alvo = procurarObjeto();
  //Serial.print ("leitura: ");
  //Serial.println (lerDistancia ());
 
  //if (alvo < 80) {

     //Serial.println("Virar ESQUERDA");

  //} 
  //else if (alvo > 100) {

     //Serial.println("Virar DIREITA");

  //} 
  //else {

    //Serial.println("Seguir RETO");
  //}

   //delay(1000);
  

}

void esquerda (int velo, int tempo) 
{
  temporizador (1,0);

  while (cronometro < tempo){
    int ultimoError;
    temporizador (0,1);
    Serial.print ("Tempo: ");
    Serial.print (cronometro);
    int error = obterAngulo ();
    Serial.print (" Error: ");
    Serial.print (obterAngulo());
    int correcaoP = Kp * error;
    int correcaoD = Kd * (ultimoError - error);
    int correcaoFinal = correcaoP + correcaoD;
    int velDir = velo - correcaoFinal;
    int velEsq = velo + correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print (" Direita: ");
    Serial.print (velDir);
    Serial.print (" Esquerda: ");
    Serial.println (velEsq);
    analogWrite (enbEF, velEsq);
    analogWrite (enbDF, velDir);
    analogWrite (enbET, velEsq);
    analogWrite (enbDT, velDir);
    digitalWrite (motorEF1, LOW);
    digitalWrite (motorEF2, HIGH);
    digitalWrite (motorDF1, LOW);
    digitalWrite (motorDF2, HIGH);
    digitalWrite (motorET1, LOW);
    digitalWrite (motorET2, HIGH);
    digitalWrite (motorDT1, LOW);
    digitalWrite (motorDT2, HIGH);
    ultimoError = error;

  }
  digitalWrite (motorEF1, LOW);
  digitalWrite (motorEF2, LOW);
  digitalWrite (motorDF1, LOW);
  digitalWrite (motorDF2, LOW);
  digitalWrite (motorET1, LOW);
  digitalWrite (motorET2, LOW);
  digitalWrite (motorDT1, LOW);
  digitalWrite (motorDT2, LOW);
  
}

void direita (int velo, int tempo) 
{
  temporizador (1,0);
  while (cronometro < tempo)
  {
    int ultimoError;
    temporizador (0,1);
    Serial.print ("Tempo: ");
    Serial.print (cronometro);
    int error = obterAngulo ();
    Serial.print (" Error: ");
    Serial.print (obterAngulo());
    int correcaoP = Kp * error;
    int correcaoD = Kd * (ultimoError - error);
    int correcaoFinal = correcaoP + correcaoD;
    int velDir = velo + correcaoFinal;
    int velEsq = velo - correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print (" Direita: ");
    Serial.print (velDir);
    Serial.print (" Esquerda: ");
    Serial.println (velEsq);
    analogWrite (enbEF, velEsq);
    analogWrite (enbDF, velDir);
    analogWrite (enbET, velEsq);
    analogWrite (enbDT, velDir);
    digitalWrite (motorEF1, HIGH);
    digitalWrite (motorEF2, LOW);
    digitalWrite (motorDF1, HIGH);
    digitalWrite (motorDF2, LOW);
    digitalWrite (motorET1, HIGH);
    digitalWrite (motorET2, LOW);
    digitalWrite (motorDT1, HIGH);
    digitalWrite (motorDT2, LOW);
    ultimoError = error;
  }

  digitalWrite (motorEF1, LOW);
  digitalWrite (motorEF2, LOW);
  digitalWrite (motorDF1, LOW);
  digitalWrite (motorDF2, LOW);
  digitalWrite (motorET1, LOW);
  digitalWrite (motorET2, LOW);
  digitalWrite (motorDT1, LOW);
  digitalWrite (motorDT2, LOW);

}

void frente (int velo, int tempo) 
{
  temporizador (1,0);

  while (cronometro < tempo)
  {
    int ultimoError;
    temporizador (0,1);
    Serial.print ("Tempo: ");
    Serial.print (cronometro);
    int error = obterAngulo ();
    Serial.print (" Error: ");
    Serial.print (obterAngulo());
    int correcaoP = Kp * error;
    int correcaoD = Kd * (ultimoError - error);
    int correcaoFinal = correcaoP + correcaoD;
    int velDir = velo + correcaoFinal;
    int velEsq = velo - correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print (" Direita: ");
    Serial.print (velDir);
    Serial.print (" Esquerda: ");
    Serial.println (velEsq);
    analogWrite (enbEF, velDir);
    analogWrite (enbDF, velDir);
    analogWrite (enbET, velEsq);
    analogWrite (enbDT, velEsq);
    digitalWrite (motorEF1, HIGH);
    digitalWrite (motorEF2, LOW);
    digitalWrite (motorDF1, LOW);
    digitalWrite (motorDF2, HIGH);
    digitalWrite (motorET1, LOW);
    digitalWrite (motorET2, HIGH);
    digitalWrite (motorDT1, HIGH);
    digitalWrite (motorDT2, LOW);
    ultimoError = error;

  }
  digitalWrite (motorEF1, LOW);
  digitalWrite (motorEF2, LOW);
  digitalWrite (motorDF1, LOW);
  digitalWrite (motorDF2, LOW);
  digitalWrite (motorET1, LOW);
  digitalWrite (motorET2, LOW);
  digitalWrite (motorDT1, LOW);
  digitalWrite (motorDT2, LOW);

}


void tras (int velo, int tempo) 
{
  temporizador (1,0);

  while (cronometro < tempo)
  {
    int ultimoError;
    temporizador (0,1);
    Serial.print ("Tempo: ");
    Serial.print (cronometro);
    int error = obterAngulo ();
    Serial.print (" Error: ");
    Serial.print (obterAngulo());
    int correcaoP = Kp * error;
    int correcaoD = Kd * (ultimoError - error);
    int correcaoFinal = correcaoP + correcaoD;
    int velDir = velo - correcaoFinal;
    int velEsq = velo + correcaoFinal;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    Serial.print (" Direita: ");
    Serial.print (velDir);
    Serial.print (" Esquerda: ");
    Serial.println (velEsq);

    analogWrite (enbEF, velDir);
    analogWrite (enbDF, velDir);
    analogWrite (enbET, velEsq);
    analogWrite (enbDT, velEsq);
    digitalWrite (motorEF2, HIGH);
    digitalWrite (motorEF1, LOW);
    digitalWrite (motorDF2, LOW);
    digitalWrite (motorDF1, HIGH);
    digitalWrite (motorET2, LOW);
    digitalWrite (motorET1, HIGH);
    digitalWrite (motorDT2, HIGH);
    digitalWrite (motorDT1, LOW);
    ultimoError = error;
  }
  
  digitalWrite (motorEF1, LOW);
  digitalWrite (motorEF2, LOW);
  digitalWrite (motorDF1, LOW);
  digitalWrite (motorDF2, LOW);
  digitalWrite (motorET1, LOW);
  digitalWrite (motorET2, LOW);
  digitalWrite (motorDT1, LOW);
  digitalWrite (motorDT2, LOW);

}

void temporizador(int Z, int C)
{
  if(C==1){
    cronometro++;
  } else if(Z==1)
  {
    cronometro = 0;
  }
}
