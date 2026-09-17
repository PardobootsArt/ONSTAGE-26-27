
// =====================================================
// LEDS DA CÁPSULA
// =====================================================

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


// =====================================================
// HC-SR04
// =====================================================

const int trigPin = 50;
const int echoPin = 51;

float distancia;


// =====================================================
// ESTADO DOS LEDS
// =====================================================

bool estadoLed = false;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Configura todos os LEDs
  for (int i = 0; i < totalLeds; i++) {
    pinMode(pinLed[i], OUTPUT);
    digitalWrite(pinLed[i], LOW);
  }

  estadoLed = false;

  Serial.println("CAPSULA PRONTA!");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // 1. RECEBER COMANDO DO PYTHON / VS CODE
  // ===================================================

  if (Serial.available() > 0) {

    String comando = Serial.readStringUntil('\n');
    comando.trim();

    Serial.print("Comando recebido: ");
    Serial.println(comando);

    // Se o Python mandar "SUSTO"
    if (comando == "SUSTO") {

      // Liga o estado da cápsula
      estadoLed = true;

      Serial.println("SUSTO RECEBIDO!");
      Serial.println("Acendendo LEDs...");

      // Acende todos os LEDs
      for (int i = 0; i < totalLeds; i++) {
        digitalWrite(pinLed[i], HIGH);
        delay(300);
      }

      Serial.println("LEDs ACESOS!");
    }
  }


  // ===================================================
  // 2. HC-SR04 — PLANO B
  // ===================================================

  calculo();

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");


  // Se colocar a mão a menos de 10 cm
  if (distancia < 10) {

    estadoLed = !estadoLed;

    Serial.print("Estado LED: ");
    Serial.println(estadoLed);


    // Liga os LEDs
    if (estadoLed == true) {

      for (int i = 0; i < totalLeds; i++) {

        digitalWrite(pinLed[i], HIGH);
        delay(300);
      }
    }


    // Desliga os LEDs
    if (estadoLed == false) {

      for (int i = 0; i < totalLeds; i++) {

        digitalWrite(pinLed[i], LOW);
        delay(300);
      }
    }
  }


  delay(500);
}


// =====================================================
// FUNÇÃO DO HC-SR04
// =====================================================

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

