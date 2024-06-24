#include "config.hpp"
#include "H_bridge_TB6612.hpp"
#include "VL53_sensors.hpp"
#include <Arduino.h>
#include "controle_juiz.hpp"

VL53_sensors sensores;
controle_juiz controle_sony(34);


Motor motor1 = Motor(AIN1, AIN2, PWMA, STBY, offsetA, 10);
Motor motor2 = Motor(BIN1, BIN2, PWMB, STBY, offsetB, 10);

int read_ir = -1;
int last_ir = 0;

int vel_motor_1;
int vel_motor_2;

int mediaCentro;

void andar(int mot1, int mot2);
void girar();

void setup() 
{
	Serial.begin(112500);
	sensores.sensorsInit();
  controle_sony.init();
}
void loop() {
  read_ir = controle_sony.read();
  if (last_ir == TWO && (read_ir == -1))
  {
    read_ir = TWO;
  }

	sensores.distanceRead();

switch (read_ir)
{
  case ONE:
    andar(0,0);
    last_ir = ONE;
    break;
  case TWO:
    girar();
    andar(vel_motor_1,vel_motor_2);
    last_ir = TWO;
    break;
  default:
  break;
}
   read_ir = controle_sony.read();


}
void andar(int mot1, int mot2){
    motor1.drive(mot1);
    motor2.drive(mot2);
}
void girar()
{
  mediaCentro = sensores.PesosDistancias();
  if (mediaCentro == -1){
    vel_motor_1 = -200;
    vel_motor_2 = 200;
  } else {
    if (mediaCentro < -50){
      vel_motor_1 = -200;
      vel_motor_2 = 200;
    } else if (mediaCentro > 50){
      vel_motor_1 = 200;
      vel_motor_2 = -200;
    } else {
      vel_motor_1 = 0;
      vel_motor_2 = 0;
    }
  }
}



