#include <Arduino.h>
#include <BluetoothSerial.h>
#include "H_bridge_TB6612.hpp"
#include "controle_juiz.h"
#include "led_rgb.h"
#include "VL53_sensors.h"
#include <Wire.h>
#include "refletancia.h"
#include "configs.h"


float start_time = 0;
float current_time = 0;

int read_ir = -1;
int last_ir = 0;

// Objeto IR
controle_juiz controle(IR_PIN);

// Objeto VLs
VL53_sensors sensores;

// Objeto Motores
Motor rightMotor = Motor(AIN1, AIN2, PWMA, STBY, 1, 10);
Motor leftMotor = Motor(BIN1, BIN2, PWMB, STBY, 2, 10);

// Objeto led
led_rgb LED;

// Objeto QR
refletancia qr_dir(qrDir, 3700);
refletancia qr_esq(qrEsq, 3700);

float  read_sensor_dir = 0;
float read_sensor_esq = 0; 
bool border_dir, border_esq; 
int last_line_detected = 0;
int line_detected = 0;
int backwards = 0;

enum {
  S0, 
  S1,
  S2, 
  AFTER_ATTACK
};
int strategy = S0;
bool bandeira_flag = false;

void check_border();
void attack();
void printQR ();
void printQRBT ();
void strategy_selector();
void hard_stop ();

int left_vel = 0;
int right_vel = 0;

void setup() {
  Serial.begin(112500);
  SerialBT.begin(device_name); //Bluetooth device name
  LED.init();
  controle.init();
  LED.set(AZUL);
  delay(1000);
  LED.set(0);
  sensores.sensorsInit();
}

void loop() {
  read_ir = controle.read();
  if (last_ir == TWO && (read_ir == ONE || read_ir == -1))
  {
    read_ir = TWO;
  }
  
  read_sensor_dir = qr_dir.read();
  read_sensor_esq = qr_esq.read();
  border_dir = qr_dir.detect_border();
  border_esq = qr_esq.detect_border();
  // printQRBT();

  // Serial.print("Strategy: ");
  // Serial.println(strategy);

  // SerialBT.print("left_vel: ");
  // SerialBT.println(left_vel);
  // SerialBT.print("right_vel: ");
  // SerialBT.println(right_vel);

  switch (read_ir) {
    case ONE:
      //leftMotor.drive(255);
      //rightMotor.drive(-255);
      last_ir = ONE;
      LED.latch(200, VERDE);
      start_time = millis();
      break;
    
    case TWO:
      current_time = millis();
      check_border();
      sensores.distanceRead();
      // sensores.printDistancesBT();
      check_border();
      strategy_selector();
      attack();
      check_border();
      leftMotor.drive(left_vel);
      delay(10);
      rightMotor.drive(right_vel);
      delay(10);
      if(line_detected != 1)
        LED.set(MAGENTA);
      last_ir = TWO;
      break;

    case TREE:
      leftMotor.drive(0);
      delay(10);
      rightMotor.drive(0);
      delay(10);
      LED.set(VERMELHO);
      last_ir = TREE;
      start_time = millis();
      break;
    case BANDEIRA:
      bandeira_flag = true;
      LED.set(AMARELO);
      last_ir = BANDEIRA;
      start_time = millis();
      break;
    case FOUR:
      strategy = S0;
      // tempoPrevio = 0;
      last_ir = FOUR;
      LED.latch(200, AZUL);
      start_time = millis();
      break;
    case FIVE:
      strategy = S1;
      LED.latch(200,VERMELHO);
      last_ir = FIVE;
      start_time = millis();
      break;
    case SIX:
      strategy = S2;
      LED.latch(200, LARANJA);
      last_ir = SIX;
      start_time = millis();
      break;
    default:
      start_time = millis();
      LED.set(0);
      break;
  }
  read_ir = controle.read();
}

void check_border()
{
  if (line_detected != last_line_detected && backwards == 0)
  {
    start_time = millis();
  }
  last_line_detected = line_detected;
  if (border_dir && border_esq)
  {  
    if (current_time - start_time < 800)
    {
      left_vel = -900;
      right_vel = 900;
      backwards = 1;
    }
    else
    {
      left_vel = 800;
      right_vel = -300;
      backwards = 0;
    }  
    // hard_stop (); // Comentar dps
    LED.set(LARANJA);
    line_detected = 1;
  }
  else if (border_dir)
  {
    if (current_time - start_time < 600)
    {
      left_vel = -900;
      right_vel = 0;
      backwards = 1;
    }
    else
    {
      left_vel = 300;
      right_vel = 800;
      backwards = 0;
    }
    // hard_stop (); // Comentar dps
    LED.set(LARANJA);
    line_detected = 1;
  }
  else if (border_esq)
  {
    if (current_time - start_time < 600)
    {
      left_vel = 0;
      right_vel = 900;
      backwards = 1;
    }
    else
    {
      left_vel = 800;
      right_vel = 300;
      backwards = 0;
    }
    // hard_stop (); // Comentar dps
    LED.set(LARANJA);
    line_detected = 1;
  }
  else
  {  
    line_detected = 0;
    LED.set(MAGENTA);
  }
}

void printQR ()
{
  Serial.print(" ");
  Serial.print("read_sensor_dir:");
  Serial.print(" ");
  Serial.print(read_sensor_dir);
  Serial.print(" ");
  Serial.print("bool dir:");
  Serial.print(" ");
  Serial.print(border_dir);
  Serial.println("\t\t");
  Serial.print(" ");
  Serial.print("read_sensor_esq");
  Serial.print(" ");
  Serial.print(read_sensor_esq);
  Serial.print(" ");
  Serial.print("bool esq:");
  Serial.print(" ");
  Serial.print(border_esq);
  Serial.println("\t\t");
}

void printQRBT ()
{
  SerialBT.print(" ");
  SerialBT.print("read_sensor_dir:");
  SerialBT.print(" ");
  SerialBT.print(read_sensor_dir);
  SerialBT.print(" ");
  SerialBT.print("bool dir:");
  SerialBT.print(" ");
  SerialBT.print(border_dir);
  SerialBT.println("\t\t");
  SerialBT.print(" ");
  SerialBT.print("read_sensor_esq");
  SerialBT.print(" ");
  SerialBT.print(read_sensor_esq);
  SerialBT.print(" ");
  SerialBT.print("bool esq:");
  SerialBT.print(" ");
  SerialBT.print(border_esq);
  SerialBT.println("\t\t");
}

void attack()
{
  if(sensores.dist[1] <= 50 && sensores.dist[2] <= 50 && bandeira_flag == 0)
  {
    left_vel = 950;
    right_vel = -950;
  }
  else if(sensores.dist[1] <= 100 && sensores.dist[2] <= 100 && bandeira_flag == 0)
  {
    left_vel = 700;
    right_vel = -700;
  }
  else if(sensores.dist[1] <= 200 && sensores.dist[0] <= 100 && bandeira_flag == 0)
  {
    left_vel = 700;
    right_vel = -450;
  }
  else if(sensores.dist[2] <= 200 && sensores.dist[3] <= 100 && bandeira_flag == 0)
  {
    left_vel = 450;
    right_vel = -700;
  }
  else if (bandeira_flag)
  {
    if (sensores.dist[1] <= 50 && sensores.dist[2] <= 50 && sensores.dist[3] <= 80 && sensores.dist[0] <= 80)
    {
      left_vel = 950;
      right_vel = -950;
    }
    else if(sensores.dist[1] <= 100 && sensores.dist[2] <= 100 && sensores.dist[3] <= 200 && sensores.dist[0] <= 200)
    {
      left_vel = 700;
      right_vel = -700;
    }
  }
}

void strategy_selector()
{
  current_time = millis();
  if (strategy == S0)
  {      
    left_vel = 200;
    right_vel = -200;
  }
  if (strategy == S1)
  {
      left_vel = 250;
      right_vel = -200;
  }
  if (strategy == S2)
  {
    left_vel = 200;
    right_vel = -250;
  }
  if (strategy == AFTER_ATTACK)
  {
    left_vel = 400;
    right_vel = -200;
  }
  // else
  // {
  //   left_vel = 0;
  //   right_vel = 0;
  // }
}
void hard_stop ()
{
  leftMotor.drive(0); // Comentar dps
  delay(10); // Comentar dps
  rightMotor.drive(0); // Comentar dps
  delay(10); // Comentar dps
  left_vel = 0; // Comentar dps
  right_vel = 0; // Comentar dps
}