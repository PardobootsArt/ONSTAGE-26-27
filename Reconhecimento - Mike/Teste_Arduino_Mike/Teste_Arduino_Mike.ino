//=====================================================
// VARIÁVEIS RECEBIDAS DO PYTHON
//=====================================================

// ID da Apriltag detectada.
// Se for -1 significa que nenhuma tag foi encontrada.
int tagID = -1;

// Erro horizontal em pixels.
float erroX = 0;

// Erro de distância.
float erroDistancia = 0;

float erroPerspectiva = 0;


//=====================================================
// GANHOS DO CONTROLADOR
//=====================================================

float KpX = 0.5;     // Controle lateral
float KpDist = 0.6;  // Controle da distância
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

int enbEF = 11;
int enbDF = 10;
int enbET = 13;
int enbDT = 12;

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

      erroPerspectiva = mensagem.substring(terceiraVirgula + 1).toFloat();
    }

    if (tagID == 0) {

      int velocidadeBase = 80;
      // Controle proporcional

      Vx = velocidadeBase + (KpDist * erroDistancia);
      W = KpX * erroX;
      // Limita o PWM
      float velEF = Vx - W;
      float velDF = Vx + W;
      float velET = Vx + W;
      float velDT = Vx - W;

      velEF = constrain(velEF, 0, 255);
      velDF = constrain(velDF, 0, 255);
      velET = constrain(velET, 0, 255);
      velDT = constrain(velDT, 0, 255);

      digitalWrite(motorEF1, LOW);
      digitalWrite(motorEF2, HIGH);
      analogWrite(enbEF, velEF);

      digitalWrite(motorDF1, HIGH);
      digitalWrite(motorDF2, LOW);
      analogWrite(enbDF, velDF);

      digitalWrite(motorET1, HIGH);
      digitalWrite(motorET2, LOW);
      analogWrite(enbET, velET);

      digitalWrite(motorDT1, HIGH);
      digitalWrite(motorDT2, LOW);
      analogWrite(enbDT, velDT);
    }

    if (tagID == 1) {
      digitalWrite(motorEF1, LOW);
      digitalWrite(motorEF2, LOW);

      digitalWrite(motorDF1, LOW);
      digitalWrite(motorDF2, LOW);

      digitalWrite(motorET1, LOW);
      digitalWrite(motorET2, LOW);

      digitalWrite(motorDT1, LOW);
      digitalWrite(motorDT2, LOW);
    }

    if (tagID == 2) {
      Vy = (KpPersp * erroPerspectiva);
      if (erroPerspectiva > 0) {
        float velEF = 75 - Vy;
        float velDF = 75 - Vy;
        float velET = 75 + Vy;
        float velDT = 75 +Vy;

        velEF = constrain(velEF, 0, 255);
        velDF = constrain(velDF, 0, 255);
        velET = constrain(velET, 0, 255);
        velDT = constrain(velDT, 0, 255);

        digitalWrite(motorEF1, LOW);
        digitalWrite(motorEF2, HIGH);
        analogWrite(enbEF, velEF);

        digitalWrite(motorDF2, HIGH);
        digitalWrite(motorDF1, LOW);
        analogWrite(enbDF, velDF);

        digitalWrite(motorET2, HIGH);
        digitalWrite(motorET1, LOW);
        analogWrite(enbET, velET);

        digitalWrite(motorDT1, HIGH);
        digitalWrite(motorDT2, LOW);
        analogWrite(enbDT, velDT);
      }
    }
  }
}
