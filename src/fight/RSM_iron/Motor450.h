// Motor450.h
// rev 1 - Sep 2023 - shabaz - converted to an Arduino library

#ifndef MOTOR450_H
#define MOTOR450_H

#include "Arduino.h"

// defines
#define PWMWRITE(channel, value) ledcWrite(channel, value)
// use .stop(HARD_BRAKE) to stop the motor quickly
// otherwise, use .stop() to coast to a stop.
#define HARD_BRAKE 1


// class definition
class Motor450
{
  private:
    int _channel1;
    int _channel2; 
    int _resolution;
    byte mDriverPin1, mDriverPin2;
  public:
    // Motor450 objects require two PWM-capable pins
    Motor450(byte driverPin1, byte driverPin2, int channel1, int channel2, int resolution);
    // Run the motor with direction [0..1] and speed [0..255]
    void drive(int speed);
    // Stop the motor. If brake value is 1 then motor is rapidly halted
    void stop(byte brake);
    void stop();

};

#endif // MOTOR450_H
