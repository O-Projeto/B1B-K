#include <VL53L0X.h> 
#pragma once 

#define SCL_SDIST 22
#define SDA_SDIST 21

#define SDIST_DR 16 //change the pins  
#define SDIST_C_DR 17 
#define SDIST_C_ESQ 18
#define SDIST_ESQ 19
#define NUM_SENSORS 4

// #define B1B_K_1_Led
#define B1B_K_3_Led

class VL53_sensors
{
private:
    /* 0 - Direita */
public:
    int number_sensor = NUM_SENSORS; // change this value 
    int x_shut_pins[NUM_SENSORS] = {SDIST_DR, SDIST_C_DR, SDIST_C_ESQ, SDIST_ESQ} ; 
    int dist[NUM_SENSORS];
    
    // Com trash holder 
    #ifdef B1B_K_3_Led
        int offset[NUM_SENSORS] = {0, -25, 0, -70}; //             
    #endif

    #ifdef B1B_K_1_Led
        int offset[NUM_SENSORS] = {0, -20, 0, 0}; // 
    #endif


    VL53L0X sensor[NUM_SENSORS];
    
    void sensorsInit();
    void distanceRead();
    void printDistances();
    void printDistancesSensor(int sensor_num);
    int PesosDistancias ();
};
