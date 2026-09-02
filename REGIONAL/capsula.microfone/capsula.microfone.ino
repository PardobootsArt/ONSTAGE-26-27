// ============================================================
// LEDS
// ============================================================

const int pinLed[] = {
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  32,
  33,
  34,
  35,
  36,
  37,
  38,
  39,
  40,
  41,
  42,
  43,
  44
};

const int totalLeds = sizeof(pinLed) / sizeof(pinLed[0]);


// ============================================================
// SENSOR HC-SR04
// ============================================================

const int trigPin = 50;
const int echoPin = 51;

float distancia;
bool estadoLed;


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Configura os LEDs
  for (int i = 0; i < totalLeds; i++) {

    pinMode(pinLed[i], OUTPUT);

    digitalWrite(pinLed[i], LOW);
  }

  estadoLed = false;
}


// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // 1. Verifica se chegou algum comando do computador
  // ----------------------------------------------------------

  verificarSerial();


  // ----------------------------------------------------------
  // 2. Calcula a distância do sensor
  // ----------------------------------------------------------

  calculo();

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");


  // ----------------------------------------------------------
  // 3. Comportamento original do sensor
  // ----------------------------------------------------------

  if (distancia < 10) {

    estadoLed = !estadoLed;

    if (estadoLed == true) {

      acenderLeds();
    }

    else {

      apagarLeds();
    }
  }


  delay(500);
}


// ============================================================
// RECEBE COMANDOS DO PYTHON
// ============================================================

void verificarSerial() {

  if (Serial.available() > 0) {

    String comando = Serial.readStringUntil('\n');

    comando.trim();


    // --------------------------------------------------------
    // COMANDO: SUSTO
    // --------------------------------------------------------

    if (comando == "SUSTO") {

      Serial.println(">>> SUSTO DETECTADO PELO COMPUTADOR!");

      acenderLeds();

      // Mantém o estado como ligado
      estadoLed = true;
    }


    // --------------------------------------------------------
    // COMANDO: DESLIGAR
    // --------------------------------------------------------

    else if (comando == "DESLIGAR") {

      apagarLeds();

      estadoLed = false;
    }
  }
}


// ============================================================
// ACENDER TODOS OS LEDS
// ============================================================

void acenderLeds() {

  for (int i = 0; i < totalLeds; i++) {

    digitalWrite(pinLed[i], HIGH);

    delay(300);
  }
}


// ============================================================
// APAGAR TODOS OS LEDS
// ============================================================

void apagarLeds() {

  for (int i = 0; i < totalLeds; i++) {

    digitalWrite(pinLed[i], LOW);

    delay(300);
  }
}


// ============================================================
// CALCULA DISTÂNCIA DO HC-SR04
// ============================================================

void calculo() {

  digitalWrite(trigPin, LOW);

  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);


  // Lê o retorno
  long duracao = pulseIn(echoPin, HIGH);


  // Converte para centímetros
  distancia = duracao * 0.0343 / 2;
}