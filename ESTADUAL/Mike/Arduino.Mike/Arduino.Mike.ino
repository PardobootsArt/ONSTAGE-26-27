#include <Servo.h>
//olho
Servo horizontal;
Servo vertical;

//braços
Servo direito;   //braco direito
Servo esquerdo;  //braco esquerdo
bool bracosLevantados = false;

// prototipos das funcoes
void movimentacaoolho();
void olho(int vertical2, int horizontal2);
void levantarBracos();
void girar(int lado, unsigned long tempo);

//=====================================================
// VARIAVEIS RECEBIDAS DO PYTHON
//=====================================================
// ID da Apriltag detectada.
// Se for -1 significa que nenhuma tag foi encontrada.
int tagID = -1;

// Erro horizontal em pixels.
float erroX = 0;

// Erro de distancia.
float erroDistancia = 0;

float erroPerspectiva = 0;

int PosicaoOlho = 0;

//=====================================================
// GANHOS DO CONTROLADOR
//=====================================================

float KpX = 0.2;     // Controle lateral
float KpDist = 0.6;  // Controle da distancia
float KpPersp = 100;


//=====================================================
// VELOCIDADES CALCULADAS
//=====================================================

float Vx = 0;

float Vy = 0;

float W = 0;

//=====================================================
// PORTAS MOTORES
//=====================================================

int motorDF1 = 22;
int motorDF2 = 24;
int motorEF1 = 26;
int motorEF2 = 28;
int motorDT1 = 30;
int motorDT2 = 32;
int motorET1 = 34;
int motorET2 = 36;

int enbEF = 12;
int enbDF = 13;
int enbET = 11;
int enbDT = 10;
bool repBraco1 = false;
bool repBraco2 = false;

bool repTag2 = false;
bool repTag4 = false;
bool repTag5 = false;

void setup() {
  Serial.begin(9600);

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
  pinMode(13, OUTPUT);

  horizontal.attach(3);
  vertical.attach(2);
}
//bracos:
void falarOi() {
  direito.attach(4);
  esquerdo.attach(5);
  esquerdo.write(180);
  direito.write(0);

  direito.write(120);
  delay(500);
  direito.write(180);
  delay(500);
  direito.write(120);
  delay(500);
  direito.write(180);
  delay(500);
  direito.write(120);
  delay(500);
  direito.write(180);
  delay(500);

  direito.write(0);
}
void animado() {
  if (repBraco1 == false) {
    repBraco1 = true;
    direito.attach(4);
    esquerdo.attach(5);
    esquerdo.write(180);
    delay(150);
    direito.write(0);
    delay(500);
    direito.write(180);
    delay(150);
    esquerdo.write(0);
    delay(500);
    direito.write(90);
    delay(150);
    esquerdo.write(90);
  }
}
void girar(int lado, unsigned long tempo) {
  // lado = 1  -> gira para a direita
  // lado = -1 -> gira para a esquerda

  if (lado == 1) {
    // ==============================
    // GIRAR PARA A DIREITA
    // ==============================

    // Lado esquerdo para frente
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    // Lado direito para trás
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, HIGH);

    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
  }

  else if (lado == -1) {
    // ==============================
    // GIRAR PARA A ESQUERDA
    // ==============================

    // Lado esquerdo para tras
    digitalWrite(motorEF1, HIGH);
    digitalWrite(motorEF2, LOW);

    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, HIGH);

    // Lado direito para frente
    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);

    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
  }
  // Liga todos os motores na velocidade desejada
  analogWrite(enbEF, 100);
  analogWrite(enbDF, 100);
  analogWrite(enbET, 100);
  analogWrite(enbDT, 100);

  // Mantém o robô girando pelo tempo determinado
  delay(tempo);

  // ==============================
  // PARA TODOS OS MOTORES
  // ==============================
  analogWrite(enbEF, 0);
  analogWrite(enbDF, 0);
  analogWrite(enbET, 0);
  analogWrite(enbDT, 0);
}

void loop() {
  // erroDistancia já veio do Python
  if (Serial.available()) {
    // Lê uma linha inteira enviada pelo Python
    String mensagem = Serial.readStringUntil('\n');

    mensagem.trim();

    // Procura as vírgulas
    int primeiraVirgula = mensagem.indexOf(',');

    int segundaVirgula = mensagem.indexOf(',', primeiraVirgula + 1);

    int terceiraVirgula = mensagem.indexOf(',', segundaVirgula + 1);

    // Se encontrou todas as vírgulas...
    if (primeiraVirgula != -1 && segundaVirgula != -1 && terceiraVirgula != -1) {
      // Separa cada informação

      tagID = mensagem.substring(0, primeiraVirgula).toInt();

      erroX = mensagem.substring(primeiraVirgula + 1,
                                 segundaVirgula)
                .toFloat();

      erroDistancia = mensagem.substring(segundaVirgula + 1,
                                         terceiraVirgula)
                        .toFloat();

      PosicaoOlho = mensagem.substring(terceiraVirgula + 1).toInt();
    }

    if (tagID == 0) {
      int velocidadeBase = 40;
      // Controle proporcional
      repTag2=false;
      repTag4=false;
      repTag5=false;

      Vx = velocidadeBase + (KpDist * erroDistancia);
      W = KpX * erroX;
      // Limita o PWM
      float velEF = Vx + W;
      float velDF = Vx - W;
      float velET = Vx - W;
      float velDT = Vx + W;

      velEF = constrain(velEF, 0, 255);
      velDF = constrain(velDF, 0, 255);
      velET = constrain(velET, 0, 255);
      velDT = constrain(velDT, 0, 255);
      analogWrite(enbEF, velEF);
      analogWrite(enbDF, velDF);
      analogWrite(enbET, velET);
      analogWrite(enbDT, velDT);

      digitalWrite(motorEF1, HIGH);
      digitalWrite(motorEF2, LOW);

      digitalWrite(motorDF1, LOW);
      digitalWrite(motorDF2, HIGH);

      digitalWrite(motorET1, LOW);
      digitalWrite(motorET2, HIGH);

      digitalWrite(motorDT1, LOW);
      digitalWrite(motorDT2, HIGH);
      delay(100);
    }

    if (tagID == -1) {

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

    if (tagID == 2 && repTag2 == false) {
      repTag2 = true;
      girar(-1, 1300);
      delay(500);
    }

    if (tagID == 3) {
      movimentacaoolho();
      switch (PosicaoOlho) {
        case 1:
          olho(0, 90);
          break;
        case 2:
          olho(45, 90);
          break;
        case 3:
          olho(90, 90);
          break;
        case 4:
          olho(0, 50);
          break;
        case 5:
          olho(45, 50);
          break;
        case 6:
          olho(90, 50);
          break;
        case 7:
          olho(0, 30);
          break;
        case 8:
          olho(45, 30);
          break;
        case 9:
          olho(90, 30);
          break;
      }
    }
    if (tagID == 4 && repTag4 == false) {
      repTag4 = true;
      falarOi();
      delay(500);
    }
    if (tagID == 5 && repTag5 == false) {
      repTag5 = true;

      direito.attach(4);
      esquerdo.attach(5);
      esquerdo.write(180);
      delay(150);
      direito.write(0);
      delay(500);
      direito.write(180);
      delay(150);
      esquerdo.write(0);
      delay(500);
      direito.write(90);
      delay(150);
      esquerdo.write(90);
      delay(500);
      direito.write(180);
      delay(150);
      esquerdo.write(0);
      delay(500);
      direito.write(90);
      delay(150);
      esquerdo.write(90);
      delay(500);
      direito.write(180);
      delay(150);
      esquerdo.write(0);
      delay(500);
      direito.write(90);
      delay(150);
      esquerdo.write(90);
      delay(500);

      esquerdo.write(180);
      delay(150);
      direito.write(0);
    }
  }
}
// lado = 1  -> gira para a direita
// lado = -1 -> gira para a esquerda

void olho(int vertical2, int horizontal2) {
  vertical.write(vertical2);
  horizontal.write(horizontal2);
}
void movimentacaoolho() {
  switch (PosicaoOlho) {
    case 1:
      olho(0, 90);
      break;
    case 2:
      olho(45, 90);
      break;
    case 3:
      olho(90, 90);
      break;
    case 4:
      olho(0, 50);
      break;
    case 5:
      olho(45, 50);
      break;
    case 6:
      olho(90, 50);
      break;
    case 7:
      olho(0, 30);
      break;
    case 8:
      olho(45, 30);
      break;
    case 9:
      olho(90, 30);
      break;
  }
}