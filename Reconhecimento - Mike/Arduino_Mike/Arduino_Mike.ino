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

float KpX = 0.3;      // Controle lateral
float KpDist = 0.25;  // Controle da distância


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
  //direita
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
  //direita
  if (direcao == 1) {
    //frente
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorEF1, HIGH);

    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);

    digitalWrite(motorET2, HIGH);
    digitalWrite(motorET1, LOW);

    digitalWrite(motorDT2, LOW);
    digitalWrite(motorDT1, HIGH);
  }
  if (direcao == 2) {

    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
  }
  if (direcao == 3) {
    //direita
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);

    digitalWrite(motorET2, HIGH);
    digitalWrite(motorET1, LOW);

    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
  }
  if (direcao == 4) {
    //esquerda
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorEF1, HIGH);

    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    digitalWrite(motorDT2, LOW);
    digitalWrite(motorDT1, HIGH);
  }
  if (direcao == 180) {
    //esquerda girar 180
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    digitalWrite(motorDT2, LOW);
    digitalWrite(motorDT1, HIGH);
  }
  if (direcao == -90) {
    //esquerda girar 90
    digitalWrite(motorEF1, LOW);
    digitalWrite(motorEF2, HIGH);

    digitalWrite(motorDF2, HIGH);
    digitalWrite(motorDF1, LOW);

    digitalWrite(motorET1, HIGH);
    digitalWrite(motorET2, LOW);

    digitalWrite(motorDT2, LOW);
    digitalWrite(motorDT1, HIGH);
  }
  if (direcao == 90) {
    //direita girar 90
    digitalWrite(motorEF2, LOW);
    digitalWrite(motorEF1, HIGH);

    digitalWrite(motorDF1, HIGH);
    digitalWrite(motorDF2, LOW);

    digitalWrite(motorET2, HIGH);
    digitalWrite(motorET1, LOW);

    digitalWrite(motorDT1, LOW);
    digitalWrite(motorDT2, HIGH);
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


//direita
void loop() {
  //direita
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

      analogWrite(enbEF, velEF);
      analogWrite(enbDF, velDF);
      analogWrite(enbET, velET);
      analogWrite(enbDT, velDT);

      direcaoMotores(1);

      delay(100);
    }

    if (tagID == 1) {
      float Kpx = 0.3;
      float Kpdist = 0.25;
      if (abs(erroX) > toleranciaX) {
        Vx = 0;
        W = Kpx * abs(erroX);
        float velocid = 90 + W;
        velocid = constrain(velocid, 0, 255);
        analogWrite(enbEF, velocid);
        analogWrite(enbDF, velocid);
        analogWrite(enbET, velocid);
        analogWrite(enbDT, velocid);
        if (erroX > 0) {
          direcaoMotores(3);
        }
        if (erroX < 0) {
          direcaoMotores(4);
        }
      }
      direcaoMotores(-1);
    }
    //===============GIRAR 180 GRAUS ====================
    if (tagID == 2) {
      int velBase = 150;
      if (girar180 == false) {
        analogWrite(enbEF, velBase);
        analogWrite(enbDF, velBase);
        analogWrite(enbET, velBase);
        analogWrite(enbDT, velBase);
        direcaoMotores(180);
        delay(1600);
        direcaoMotores(-1);
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
        direcaoMotores(90);
        delay(800);
        direcaoMotores(-1);
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
        direcaoMotores(-90);
        delay(800);
        direcaoMotores(-1);
        girar90E = true;
      }
    }
    if (tagID == -1) {
      direcaoMotores(-1);
    }
  }
}
