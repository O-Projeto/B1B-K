#include <VL53L0X.h> 
#pragma once 

#define SCL_SDIST 22
#define SDA_SDIST 21

#define SDIST_DR 16 //change the pins  
#define SDIST_C_DR 17 
#define SDIST_C_ESQ 18
#define SDIST_ESQ 19
#define NUM_SENSORS 4

class VL53_sensors
{
private:
    /* data */
public:
    int number_sensor = NUM_SENSORS; // change this value 
    int x_shut_pins[NUM_SENSORS] = {SDIST_DR, SDIST_C_DR, SDIST_C_ESQ, SDIST_ESQ} ; 
    int dist[NUM_SENSORS];
    int offset[NUM_SENSORS] = {0, 0, 0, 0}; // ???

    VL53L0X sensor[NUM_SENSORS];
    
    void sensorsInit();
    void distanceRead();
    void printDistances();
    void printDistancesSensor(int sensor_num);
    int PesosDistancias ();
};
