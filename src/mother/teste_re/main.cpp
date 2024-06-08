#include "H_bridge_TB6612.hpp"
#include "config.h"
#include "refletancia.h"
#include <Arduino.h>


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
int backwards = 0;
float start_time = 0;
float current_time = 0;
float tempoRe=0;
bool flagRe= 0;

int vel_motor_1;
int vel_motor_2;

refletancia qr_dir(qrDir, 2700);
refletancia qr_esq(qrEsq, 2700);

void check_border();
void attack();
void re();
void andar(int mot1, int mot2);

void setup() {

  Serial.begin(115200);
}


void loop (){

    read_sensor_dir = qr_dir.read();
    read_sensor_esq = qr_esq.read();
    border_dir = qr_dir.detect_border();
    border_esq = qr_esq.detect_border();

    check_border();
    attack();
    check_border();
    re();
       // set 800ms tempo de re
    //atacar
      //  re é 0
        //andar pra frente
    andar(vel_motor_1,vel_motor_2);
}

void andar(int mot1, int mot2){
    motor1.drive(mot1);
    motor2.drive(mot2);
}

void re(){

    if(current_time- start_time > tempoRe && flagRe){
        vel_motor_1 = -1000;
        vel_motor_2 = -1000;
    } else{        
        tempoRe = 0;
        flagRe = 0;
    }
}
void check_border()
{
  if (line_detected != last_line_detected && backwards == 0)
  {
    start_time = millis();
  }
    last_line_detected = line_detected;
  if (border_dir || border_esq){
    tempoRe = 800;
    line_detected = 1;
    flagRe = 1;
  }
  }
void attack(){
        vel_motor_1 = 500;
        vel_motor_2 = 500;
}