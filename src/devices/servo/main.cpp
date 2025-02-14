#include <Arduino.h>
#include <ESP32Servo.h>  
#include "controle_juiz.hpp"
#include "config.hpp"

controle_juiz controle_sony(34);
Servo myServo;  

int read_ir = 0;
int last_ir = 0;
uint8_t servoAngle = 100;  // Começa em 90°
int servoMoved = 0;  // Flag para evitar movimentos repetidos

void setup() {  
  Serial.begin(115200);
  controle_sony.init();
  
  myServo.attach(SERVO);  
  myServo.write(90);  // Inicia em 90°
}

void loop() {
  read_ir = controle_sony.read();

  if (last_ir == FIVE && read_ir == SIX) {
    read_ir = TWO;
  } else if (last_ir == SIX && read_ir == FIVE) {
    read_ir = TWO;
  }

  switch (read_ir) {

    case FIVE:
      myServo.write(180);
      break;
    
    case SIX:
      myServo.write(0);
      break;
    case TWO:
      myServo.write(90);
    break;
  }
}
