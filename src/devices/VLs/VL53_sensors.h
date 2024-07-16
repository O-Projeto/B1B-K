#include <VL53L0X.h> 

#define SCL_SDIST 22
#define SDA_SDIST 21

#define SDIST_1 16 //change the pins  
#define SDIST_2 17 
#define SDIST_3 18
#define SDIST_4 19
#define NUM_SENSORS 4


//#define B1B_K_1_Led
#define B1B_K_3_Led

class VL53_sensors
{
private:
    /* 0 - Direita */
public:
    int number_sensor = NUM_SENSORS; // change this value 
    int x_shut_pins[NUM_SENSORS] = {SDIST_1, SDIST_2, SDIST_3, SDIST_4} ; 
    int dist[NUM_SENSORS];

        // Com trash holder 
    #ifdef B1B_K_3_Led
        int offset[NUM_SENSORS] = {-62, -23, 0, 0}; //             
    #endif

    #ifdef B1B_K_1_Led
        int offset[NUM_SENSORS] = {0, -31, 0, -20}; // 
    #endif

    VL53L0X sensor[NUM_SENSORS];
    
    void sensorsInit();
    void distanceRead();
    void printDistances();
    void printDistancesSensor(int sensor_num);
    int PesosDistancias();
};
