#pragma once
#include <Arduino.h>
#define NUM_SENSORS 1
#define SREAD_1 4

class JS40F_JSumo {
public:
 int number_sensor = NUM_SENSORS;
 int sensorRead [NUM_SENSORS];
 int sensorPin [NUM_SENSORS] = {SREAD_1};

 
    void sensorsInit();
    void distanceRead();
    void printDistances();
};