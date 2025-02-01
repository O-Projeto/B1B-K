#pragma once
#include <Arduino.h>
#define NUM_SENSORS 5
#define SREAD_1 4
#define SREAD_2 16
#define SREAD_3 17
#define SREAD_4 18
#define SREAD_5 19

class JS40F_JSumo {
public:
 int number_sensor = NUM_SENSORS;
 int sensorRead [NUM_SENSORS];
 int sensorPin [NUM_SENSORS] = {SREAD_1, SREAD_2, SREAD_3, SREAD_4, SREAD_5};

 
    void sensorsInit();
    void distanceRead();
    void printDistances();
};