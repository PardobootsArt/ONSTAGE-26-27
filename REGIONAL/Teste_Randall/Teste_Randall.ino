//Programa: Sensor TCS3200 e led RGB
//Autor: FILIPEFLOP

// Mais informações: https://www.filipeflop.com/blog/sensor-de-cor-tcs3200-rgb-arduino/

//Pinos de conexao do modulo
const int s0 = 40;
const int s1 = 42;
const int s2 = 44;
const int s3 = 46;
const int out = 41;

//Pinos do led RGB
int Pin_RED = 3;
int Pin_GREEN = 9;
int Pin_BLUE = 7;

//Variaveis cores
int red = 0;
int green = 0;
int blue = 0;
bool camuflagemRandall = false;

void setup() {
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
    delay(500);
    if (green > 300 && green < 500 && red > 800 && green < blue) {
      camuflagemRandall = true;
      Serial.println("Iniciando Camuflagem Randall...");
      delay(50);
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
  delay(500);
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