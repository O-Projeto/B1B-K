#include <Arduino.h>
#include "controle_juiz.h"

#define ONE 0
#define TWO 1
#define TREE 2
#define POWER 21


controle_juiz controle_sony(34);

// pra que isso serve????
int last_button_pressed = 300; 
int button_pressed = 300;

void setup() { // inicializa o controle 
  Serial.begin(112500);
  controle_sony.init();
}
void loop() {
  // printa o que foir apertado
  Serial.println(controle_sony.read());
   
}