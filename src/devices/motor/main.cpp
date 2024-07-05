/******************************************************************************
TestRun.ino
TB6612FNG H-Bridge Motor Driver Example code
Michelle @ SparkFun Electronics
8/20/16
https://github.com/sparkfun/SparkFun_TB6612FNG_Arduino_Library

Uses 2 motors to show devices of the functions in the library.  This causes
a robot to do a little 'jig'.  Each movement has an equal and opposite movement
so assuming your motors are balanced the bot should end up at the same place it
started.

Resources:
TB6612 SparkFun Library

Development environment specifics:
Developed on Arduino 1.6.4
Developed with ROB-9457
******************************************************************************/

// This is the library for the TB6612 that contains the class Motor and all the
// functions
#include "H_bridge_TB6612.hpp"

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
// the default pins listed are the ones used on the Redbot (ROB-12097) with
// the exception of STBY which the Redbot controls with a physical switch
#define AIN1 25
#define BIN1 27
#define AIN2 33
#define BIN2 14
#define PWMA 32
#define PWMB 13
#define STBY 26

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 2;

// Initializing motors.  The library will allow you to initialize as many
// motors as you have memory for.  If you are using functions like forward
// that take 2 motors as arguements you can either write new functions or
// call the function more than once.
Motor motor1 = Motor(AIN1, AIN2, PWMA, STBY, offsetA, 10);
Motor motor2 = Motor(BIN1, BIN2, PWMB, STBY, offsetB, 10);

void setup() {

  Serial.begin(9600);
  

}

void loop() {

    motor1.drive(500);
    motor2.drive(500);
    
    delay(2000);

    // motor1.drive(-500);
    // motor2.drive(-500);

    // delay(2000);
    
  // //PWM for the right motor
  // for(int i=0; i< 1000; i++) {
  //   motor1.drive(i);
  //   Serial.println(i);
  //   delay(10);
  // }

  // for(int j = 1000; j > - 1000; j--) {
  //   motor1.drive(j);
  //   Serial.println(j);
  //   delay(10);
  // }

  // for(int k = -1000; k < 0; k++) {
  //   motor1.drive(k);
  //   Serial.println(k);
  //   delay(10);
  // }

  //  //PWM for the motor2 motor
  // for(int i=0; i< 1000; i++) {
  //   motor2.drive(i);
  //   Serial.println(i);
  //   delay(10);
  // }

  // for(int j = 1000; j > - 1000; j--) {
  //   motor2.drive(j);
  //   Serial.println(j);
  //   delay(10);
  // }

  // for(int k = -1000; k < 0; k++) {
  //   motor2.drive(k);
  //   Serial.println(k);
  //   delay(10);
  // }

  // //PWM for the motor1 and motor 2 motor
  // for(int i=0; i< 1000; i++) {
  //   motor1.drive(i);
  //   motor2.drive(i);
  //   Serial.println(i);
  //   delay(10);
  // }

  // for(int j = 1000; j > - 1000; j--) {
  //   motor1.drive(j);
  //   motor2.drive(j);
  //   Serial.println(j);
  //   delay(10);
  // }

  // for(int k = -1000; k < 0; k++) {
  //   motor1.drive(k);
  //   motor2.drive(k);
  //   Serial.println(k);
  //   delay(10);
  // }
}