#include <Wire.h>
#include "Adafruit_TCS34725.h"
 
// Criação do objeto para o sensor TCS34725 com:
// - Tempo de integração de 300ms (melhor sensibilidade)
// - Ganho de 1x (evita saturação em ambientes muito iluminados)
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_300MS,
  TCS34725_GAIN_1X
);
 
void setup() {
  Serial.begin(9600);
 
  // Inicializa o sensor TCS34725
  if (!tcs.begin()) {
    Serial.println("Não foi possível encontrar o TCS34725. Verifique as conexões!");
    while (1);  // Travamento caso o sensor não seja encontrado
  }
 
  Serial.println("Sensor TCS34725 inicializado.\n");
}
 
void loop() {
  uint16_t r, g, b, c;
 
  // Lê os valores brutos do sensor (canal vermelho, verde, azul e claro)
  tcs.getRawData(&r, &g, &b, &c);
 
  // Exibe os valores lidos no Serial Monitor
  Serial.print("R: "); Serial.print(r);
  Serial.print(" | G: "); Serial.print(g);
  Serial.print(" | B: "); Serial.print(b);
  Serial.print(" | Clear: "); Serial.println(c);

   if (r > 2000 &&  r < 4000 && g > 600 && g < 800 &&  b > 500 && b < 700){
    Serial.print("Rosa| ");
  }
  if (r > 2200 &&  r < 6500 && g > 1500 && g < 5200 &&  b > 500 && b < 2000){
    Serial.print("Amarelo| ");
  }
   if (r > 600 &&  r < 1200 && g > 1000 && g < 2200 &&  b > 800 && b < 1500){
    Serial.print("Azul| ");
  }
     if (r > 200 &&  r < 300 && g > 200 && g < 500 &&  b > 100 && b < 250){
    Serial.print("Preto | ");
  }
 
  delay(50);  // Pequeno atraso para estabilidade da leitura
}