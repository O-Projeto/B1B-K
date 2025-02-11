#include <Arduino.h>
#include "led_rgb.h"

//  Definição das cores do led rgb
#define AMARELO 16768256 
#define VERMELHO 16515843
#define VERDE 63240
#define AZUL 49911
#define MAGENTA 16711935
#define CIANO 65535
led_rgb LED;


// the setup routine runs once when you press reset:
void setup() {
  LED.init();
  pinMode(2, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
     LED.blink(1000,AZUL);
     digitalWrite(2, HIGH);
     delay(500);
     digitalWrite(2, LOW);
     delay(500);
   
    // LED.latch(1000,MAGENTA);
 
}