#include "H_bridge_TB6612.hpp"
#include "config.h"
#include "refletancia.h"
#include <Arduino.h>
#include "devices/IR/controle_juiz.hpp"


// Initializing motors.  The library will allow you to initialize as many
// motors as you have memory for.  If you are using functions like forward
// that take 2 motors as arguements you can either write new functions or
// call the function more than once.
Motor motor1 = Motor(AIN1, AIN2, PWMA, STBY, offsetA, 10);
Motor motor2 = Motor(BIN1, BIN2, PWMB, STBY, offsetB, 10);

float  read_sensor_dir = 0;
float read_sensor_esq = 0; 
bool border_dir, border_esq; 
int last_line_detected = 0;
int line_detected = 0;
float start_time = 0;
float current_time = 0;
float tempoRe=0;
bool flagRe= 0;

int read_ir = -1;
int last_ir = 0;

int vel_motor_1;
int vel_motor_2;

refletancia qr_dir(qrDir, 2400);
refletancia qr_esq(qrEsq, 2400);

controle_juiz controle_sony(34);

void check_border();
void attack();
void re();
void drive(int mot1, int mot2);

void setup() {
  Serial.begin(115200);
  controle_sony.init();
}


void loop (){
  read_ir = controle_sony.read();
  if (last_ir == TWO && (read_ir == -1))
  {
    read_ir = TWO;
  }
    read_sensor_dir = qr_dir.read();
    read_sensor_esq = qr_esq.read();
    border_dir = qr_dir.detect_border();
    border_esq = qr_esq.detect_border();
    
switch (read_ir)
{
  case ONE:
    drive(0,0);
    delay(10);
    last_ir = ONE;
    break;
  case TWO:
    check_border();
    attack();
    check_border();
    re();
    drive(vel_motor_1,vel_motor_2);
    last_ir = TWO;
    break;
  default:
  break;
}
   read_ir = controle_sony.read();
}

void drive(int mot1, int mot2){
    motor1.drive(mot1);
    motor2.drive(mot2);
}

void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe && flagRe){
      if (border_dir && border_esq){
        tempoRe = 150;
        vel_motor_1 = -700;
        vel_motor_2 = -700;
      }else if (border_dir){
        tempoRe = 150;
        vel_motor_1 = -200;
        vel_motor_2 = -1000;
      }else if (border_esq){
        tempoRe = 150;
        vel_motor_1 = -1000;
        vel_motor_2 = -200;
      }else{
        vel_motor_1 = -700;
        vel_motor_2 = -700;
      }
    } else{        
        tempoRe = 0;
        flagRe = 0;
    }
}
void check_border()
{
  if (line_detected != last_line_detected)
  {
    start_time = millis();
  }
    last_line_detected = line_detected;
  if (border_dir || border_esq){
    tempoRe = 200;
    line_detected = 1;
    flagRe = 1;
  }
  else {line_detected=0;}
  }
void attack(){
    if (!flagRe){
        vel_motor_1 = 600;
        vel_motor_2 = 600;
    }
}