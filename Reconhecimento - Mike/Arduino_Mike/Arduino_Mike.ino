//=====================================================
// VARIÁVEIS RECEBIDAS DO PYTHON
//=====================================================

// ID da Apriltag detectada.
// Se for -1 significa que nenhuma tag foi encontrada.
int tagID = -1;

// Erro horizontal em pixels.y
float erroX = 0;

// Erro de distância.
float erroDistancia = 0;


//=====================================================
// GANHOS DO CONTROLADOR
//=====================================================

float KpX = 0.15;    // Controle lateral
float KpDist = 0.1;  // Controle da distância


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

int enbEF = 11;
int enbDF = 10;
int enbET = 13;
int enbDT = 12;

bool girar180 = false;
bool girar90D = false;
bool girar90E = false;
int estado;
int toleranciaX = 20;
int toleranciaDist = 10;

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
  estado = 0;
}
void direcaoMotores(int direcao) {
  if (direcao == 1) {
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorEF1, HIGH);

    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);

    digitalWrite(motorET2, HIGH);
    digitalWrite(motorET1, LOW);

    digitalWrite(motorDT2, HIGH);
    digitalWrite(motorDT1, LOW);
  }
  if (direcao == 2) {
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
  }
  if (direcao == 3) {
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);

    digitalWrite(motorET2, HIGH);
    digitalWrite(motorET1, LOW);

    digitalWrite(motorDT1, HIGH);
    digitalWrite(motorDT2, LOW);
  }
  if (direcao == 4) {
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorEF1, HIGH);

    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    digitalWrite(motorDT2, HIGH);
    digitalWrite(motorDT1, LOW);
  }
  if (direcao == -1) {
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorDF1, LOW);
    digitalWrite(motorDF2, LOW);
    digitalWrite(motorET1, LOW);
    digitalWrite(motorET2, LOW);
    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, LOW);
  }
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

    // Se encontrou as duas vírgulas...
    if (primeiraVirgula != -1 && segundaVirgula != -1) {
      // Separa cada informação

      tagID = mensagem.substring(0, primeiraVirgula).toInt();

      erroX = mensagem.substring(primeiraVirgula + 1,
                                 segundaVirgula)
                .toFloat();

      erroDistancia = mensagem.substring(segundaVirgula + 1).toFloat();
    }

    if (tagID == 0) {
      girar180 = false;
      girar90D = false;
      girar90E = false;
      int velocidadeBase = 75;
      // Controle proporcional

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

      digitalWrite(motorEF2, LOW);
      digitalWrite(motorEF1, HIGH);
      analogWrite(enbEF, velEF);

      digitalWrite(motorDF2, HIGH);
      digitalWrite(motorDF1, LOW);
      analogWrite(enbDF, velDF);

      digitalWrite(motorET2, HIGH);
      digitalWrite(motorET1, LOW);
      analogWrite(enbET, velET);

      digitalWrite(motorDT2, HIGH);
      digitalWrite(motorDT1, LOW);
      analogWrite(enbDT, velDT);

      delay(100);
    }

    if (tagID == 1) {
      float KpX = 0.5;  // Controle lateral
      float KpDist = 1.5;
      if (estado == 0) {
        Vx = 0;
        W = KpX * abs(erroX);
        float velocidF = 120 + W;
        float velocidT = 100 + W;

        velocidF = constrain(velocidF, 0, 255);
        velocidT = constrain(velocidT, 0, 255);

        analogWrite(enbEF, velocidF);
        analogWrite(enbDF, velocidF);
        analogWrite(enbET, velocidT);
        analogWrite(enbDT, velocidT);
        if (erroX > 0) {
          direcaoMotores(4);  //ESQUERDA
        }
        if (erroX < 0) {
          direcaoMotores(3);  //DIREITA
        }
        if (abs(erroX) < toleranciaX) {
          direcaoMotores(-1);
        }
      }
    }
    //===============GIRAR 180 GRAUS ====================
    if (tagID == 2) {
      int velBase = 150;
      if (girar180 == false) {
        analogWrite(enbEF, velBase);
        analogWrite(enbDF, velBase);
        analogWrite(enbET, velBase);
        analogWrite(enbDT, velBase);

        digitalWrite(motorEF2, LOW);
        digitalWrite(motorEF1, HIGH);

        digitalWrite(motorDF2, LOW);
        digitalWrite(motorDF1, HIGH);

        digitalWrite(motorET2, HIGH);
        digitalWrite(motorET1, LOW);

        digitalWrite(motorDT2, LOW);
        digitalWrite(motorDT1, HIGH);
        delay(1600);
        digitalWrite(motorEF1, LOW);
        digitalWrite(motorEF2, LOW);
        digitalWrite(motorDF1, LOW);
        digitalWrite(motorDF2, LOW);
        digitalWrite(motorET1, LOW);
        digitalWrite(motorET2, LOW);
        digitalWrite(motorDT1, LOW);
        digitalWrite(motorDT2, LOW);
        girar180 = true;
      }
    }
    //============GIRAR 90 GRAUS DIREITA===============
    if (tagID == 3) {
      int velBase = 150;
      if (girar90D == false) {
        analogWrite(enbEF, velBase);
        analogWrite(enbDF, velBase);
        analogWrite(enbET, velBase);
        analogWrite(enbDT, velBase);

        digitalWrite(motorEF1, LOW);
        digitalWrite(motorEF2, HIGH);

        digitalWrite(motorDF1, LOW);
        digitalWrite(motorDF2, HIGH);

        digitalWrite(motorET1, HIGH);
        digitalWrite(motorET2, LOW);

        digitalWrite(motorDT1, LOW);
        digitalWrite(motorDT2, HIGH);
        delay(800);
        digitalWrite(motorEF1, LOW);
        digitalWrite(motorEF2, LOW);
        digitalWrite(motorDF1, LOW);
        digitalWrite(motorDF2, LOW);
        digitalWrite(motorET1, LOW);
        digitalWrite(motorET2, LOW);
        digitalWrite(motorDT1, LOW);
        digitalWrite(motorDT2, LOW);
        girar90D = true;
      }
    }
    //=========GIRAR 90 GRAUS ESQUERDA==================
    if (tagID == 4) {
      int velBase = 150;
      if (girar90E == false) {
        analogWrite(enbEF, velBase);
        analogWrite(enbDF, velBase);
        analogWrite(enbET, velBase);
        analogWrite(enbDT, velBase);

        digitalWrite(motorEF2, LOW);
        digitalWrite(motorEF1, HIGH);

        digitalWrite(motorDF2, LOW);
        digitalWrite(motorDF1, HIGH);

        digitalWrite(motorET2, HIGH);
        digitalWrite(motorET1, LOW);

        digitalWrite(motorDT2, LOW);
        digitalWrite(motorDT1, HIGH);
        delay(800);
        digitalWrite(motorEF1, LOW);
        digitalWrite(motorEF2, LOW);
        digitalWrite(motorDF1, LOW);
        digitalWrite(motorDF2, LOW);
        digitalWrite(motorET1, LOW);
        digitalWrite(motorET2, LOW);
        digitalWrite(motorDT1, LOW);
        digitalWrite(motorDT2, LOW);
        girar90E = true;
      }
    }
    if (tagID == -1) {
      digitalWrite(motorEF1, LOW);
      digitalWrite(motorEF2, LOW);
      digitalWrite(motorDF1, LOW);
      digitalWrite(motorDF2, LOW);
      digitalWrite(motorET1, LOW);
      digitalWrite(motorET2, LOW);
      digitalWrite(motorDT1, LOW);
      digitalWrite(motorDT2, LOW);
    }
  }
}
