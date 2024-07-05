#pragma once 


// Library to control the TB6612 H bridge it has 3 control pins, A,B and PWM. 
// You need the A,B to control direction and PWM to control speed
// STD is always HIGH, this class dosnt suport it by code 
// The conection should be 

///       - PWMA         VM -  MOTOR Voltage - > 12 V
//        - AIN2         VCC - Logic Voltage -> 3.3 V
//        - AIN1         GND - 
//   3.3V - STBY         AO1 - MOTOR A
//        - BIN1         AO2-  MOTOR A 
//        - BIN2         B02 - MOTOR B 
//        - PWMB         B01 - MOTOR B
//        - GND          GND


// So you can control 2 motor with 1A max each 

#include <Arduino.h>

class Motor 
{
  private:
    int IN1, IN2, PWM, STBY, _channel, _resolution;
    
  public:

    Motor(int IN1pin, int IN2pin, int PWMpin, int STBYpin, int channel, int resolution);

    void drive(int speed);

  
};
