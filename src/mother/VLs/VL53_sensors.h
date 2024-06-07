#include <VL53L0X.h> 
#pragma once 

#define SCL_SDIST 22
#define SDA_SDIST 21

#define SDIST_1 16 //change the pins  
#define SDIST_2 17 
#define SDIST_3 18
#define SDIST_4 19
#define NUM_SENSORS 4

class VL53_sensors
{
private:
    /* data */
public:
    int number_sensor = NUM_SENSORS; // change this value 
    int x_shut_pins[NUM_SENSORS] = {SDIST_1, SDIST_2, SDIST_3, SDIST_4} ; 
    int dist[NUM_SENSORS];
    int offset[NUM_SENSORS] = {10, 0, 40, 0}; // ???

    VL53L0X sensor[NUM_SENSORS];
    
    void sensorsInit();
    void distanceRead();
    void printDistances();
    void printDistancesSensor(int sensor_num);
};
