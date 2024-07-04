#include "config.hpp"
#include "H_bridge_TB6612.hpp"
//#include <BluetoothSerial.h>
#include "VL53_sensors.hpp"
#include <Arduino.h>
#include <Wire.h>
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"

VL53_sensors sensores;
controle_juiz controle_sony(34);

refletancia qr_dir(qrDir, 2400);
refletancia qr_esq(qrEsq, 2400);

led_rgb LED;

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

int mediaCentro, lastMediaCentro;

void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();

void setup() 
{
	Serial.begin(112500);
	sensores.sensorsInit();
  controle_sony.init();
  LED.init();
  LED.set(AZUL);
  delay(1000);
  LED.set(0);
}
void loop() {
  read_ir = controle_sony.read();
  if (last_ir == TWO && (read_ir == ONE || read_ir == -1)){ read_ir = TWO;}


  read_sensor_dir = qr_dir.read();
  read_sensor_esq = qr_esq.read();
  border_dir = qr_dir.detect_border();
  border_esq = qr_esq.detect_border();

  switch (read_ir){
  case ONE:
      Serial.println(read_ir);
      last_ir = ONE;
      LED.fill(VERDE);
      start_time = millis();
      break;

  case TWO:
    sensores.distanceRead();
    check_border();
    search();
    check_border();
    re();
    sensores.printDistances();
    //totalFrente();
    drive(vel_motor_1,vel_motor_2);
    last_ir = TWO;
    break;
  case TREE:
    Serial.println(read_ir);
    drive(0,0);
    delay(10);
    LED.set(VERMELHO);
    tempoRe = 0;
    flagRe = 0;
    last_ir = TREE;
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
void search()
{
  mediaCentro = sensores.PesosDistancias();
  Serial.println(mediaCentro);
  if (mediaCentro == -1 && !flagRe){
    if (lastMediaCentro < -60){
      vel_motor_1 = 200;
      vel_motor_2 = -200;
    } else if (lastMediaCentro > 60){
      vel_motor_1 = -200;
      vel_motor_2 = 200;
    } else{
      vel_motor_1 = 200;
      vel_motor_2 = -200;
    }
  } else if (!flagRe){
    if (mediaCentro < -60){
      vel_motor_1 = 500;
      vel_motor_2 = -200;
    } else if (mediaCentro > 60){
      vel_motor_1 = -200;
      vel_motor_2 = 500;
    } else {
      vel_motor_1 = 500;
      vel_motor_2 = 500;
    }
    lastMediaCentro = mediaCentro;
  }
}

void totalFrente()
{
  if (sensores.dist[1] <= 60 && sensores.dist[2]<=60)
  {
    vel_motor_1 = 700;
    vel_motor_2 = 700;
  }
}

void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe && flagRe){
        vel_motor_1 = -500;
        vel_motor_2 = -500;
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
