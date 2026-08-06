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
    AcenderLed(200, 0, 255);

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
    if (green > 200 && green < 500 && red > 300 && green < blue) {
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
      digitalWrite (motorD2, LOW);
      delay(1000);
      digitalWrite (motorE1, LOW);
      digitalWrite (motorE2, LOW);
      digitalWrite (motorD1, LOW);
      digitalWrite (motorD2, LOW);
      delay(1000);
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
      AcenderLed(150, 0, 255);
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