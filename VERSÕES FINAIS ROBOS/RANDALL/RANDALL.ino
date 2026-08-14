const int s0 = 40;
const int s1 = 42;
const int s2 = 44;
const int s3 = 46;
const int out = 41;

//Pinos do led RGB
int Pin_RED = 7;
int Pin_GREEN = 8;
int Pin_BLUE = 9;

//Variaveis cores
int red = 0;
int green = 0;
int blue = 0;
bool camuflagemRandall = false;

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

const int trigPin2 = 5;
const int echoPin2 = 6;
// Variáveis para a duração e a distância
long durationF;
int distFrente;
long durationT;
int distTras;

int ultimoErro;
int Kp = 15;
int Kd = 20;

int velocidadeBase = 75;

int corAtual = 0;
int ultimaCor = 0;
int quantidadeCores = 0;

int corEmConfirmacao = 0;
int contadorLeituras = 0;

bool encontrouTresCores = false;


void setup() {
  // put your setup code here, to run once:

  Serial2.begin(115200);
  pinMode(motorE1, OUTPUT);
  pinMode(motorE2, OUTPUT);
  pinMode(motorD1, OUTPUT);
  pinMode(motorD2, OUTPUT);
  pinMode(enbE, OUTPUT);
  pinMode(enbD, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  Serial.begin(115200);
  //Led e sensor de cor
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);
  pinMode(Pin_RED, OUTPUT);
  pinMode(Pin_GREEN, OUTPUT);
  pinMode(Pin_BLUE, OUTPUT);
  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);
  AcenderLed(175, 0, 255);
  while (Serial2.available() == false){
  }
  Serial.println("comunicação serialOK");
  int recieve = Serial2.parseInt();
  Serial.println(recieve);
  while(recieve != 3){
  }
  delay(45000);
  analogWrite(enbE, 85);
  analogWrite(enbD, 100);
  direcaoMotores(1);
  delay(5000);
  direcaoMotores(-1);
  
}
void direcaoMotores(int direcao) {
  if (direcao == -1) {
    digitalWrite(motorE1, LOW);
    digitalWrite(motorE2, LOW);
    digitalWrite(motorD1, LOW);
    digitalWrite(motorD2, LOW);
  }
  if (direcao == 1) {
    digitalWrite(motorE1, HIGH);
    digitalWrite(motorE2, LOW);
    digitalWrite(motorD1, HIGH);
    digitalWrite(motorD2, LOW);
  }
  if (direcao == 2) {
    digitalWrite(motorE1, HIGH);
    digitalWrite(motorE2, LOW);
    digitalWrite(motorD1, LOW);
    digitalWrite(motorD2, HIGH);
  }
}

//=========================VOID LOOP===================
void loop() {
  if (encontrouTresCores) {
    analogWrite(enbE, 0);
    analogWrite(enbD, 0);
    return;
  }

  if (camuflagemRandall == false) {
    direcaoMotores(1);
    color();
    //===============CERTO CORRECAO (NAO MEXER)=========
    distFrente = calDistanciaF();
    distTras = calDistanciaT();
    int erro = (distFrente - distTras);
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
    

    Serial.print("Azul ");
    Serial.print(blue);
    Serial.print(" vermelho ");
    Serial.print(red);
    Serial.print(" verde ");
    Serial.println(green);
    Serial.print("Ultra Frente: ");
    Serial.print(distFrente);
    Serial.print("Ultra Tras: ");
    Serial.println(distTras);
    if (green > 500 && red > 500 && blue > 400) {
      Serial.println("PRETO DETECTADO");
      camuflagemRandall = true;
      AcenderLed(175, 0, 255);
      analogWrite(enbE, 65);
      analogWrite(enbD, 75);
      direcaoMotores(1);
      delay(1800);
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

  if (camuflagemRandall == true) {
    //motores
    color();
    direcaoMotores(1);
    distFrente = calDistanciaF() + 2;
    distTras = calDistanciaT();
    int erro = (distFrente - distTras);
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

    Serial.print("Azul ");
    Serial.print(blue);
    Serial.print(" vermelho ");
    Serial.print(red);
    Serial.print(" verde ");
    Serial.println(green);

    Serial.print("Quantidade de Cores: ");
    Serial.println(quantidadeCores);

    Serial.print("Ultra Frente: ");
    Serial.print(distFrente);
    Serial.print(" Ultra Tras: ");
    Serial.println(distTras);

    if (red < 90 && green < 90 && blue < 90) {

      corAtual = 1;  // BRANCO

      Serial.println("BRANCO");
      AcenderLed(150, 0, 255);
    }

    else if (red < 90 && green < 100 && blue > 100) {

      corAtual = 2;  // AMARELO

      Serial.println("AMARELO");
      AcenderLed(255, 130, 0);
    }

    else if (red < 120 && blue < 120 && green > 100) {

      corAtual = 3;  // ROSA

      Serial.println("ROSA");
      AcenderLed(255, 0, 175);
    }

    else if (blue < 130 && red > 140 && green < 170) {

      corAtual = 4;  // AZUL

      Serial.println("AZUL");
      AcenderLed(64, 224, 208);
    }

    else {
      corAtual = 0;
    }

    if (corAtual != 0 && corAtual != 1) {

      // É uma cor diferente da que estamos confirmando?
      if (corAtual != corEmConfirmacao) {

        // Começou a aparecer uma nova cor
        corEmConfirmacao = corAtual;
        contadorLeituras = 1;

      } else {
        // A mesma cor apareceu novamente
        contadorLeituras++;
      }

      // A nova cor apareceu 3 vezes seguidas
      if (contadorLeituras >= 10) {

        // Só conta se for diferente da última fileira registrada
        if (corEmConfirmacao != ultimaCor) {

          quantidadeCores++;

          ultimaCor = corEmConfirmacao;

          Serial.print("NOVA FILEIRA CONFIRMADA! Quantidade: ");
          Serial.println(quantidadeCores);
        }

        // Reseta a confirmação
        contadorLeituras = 0;
      }
    }

    if (quantidadeCores >= 3) {

      encontrouTresCores = true;
      analogWrite(enbE, 65);
      analogWrite(enbD, 65);
      delay(1000);
      analogWrite(enbE, 0);
      analogWrite(enbD, 0);

      Serial.println("3 CORES DETECTADAS - PARANDO!");
    }
  }
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
  delay(30);
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
  delay(30);
  return distTras;
}