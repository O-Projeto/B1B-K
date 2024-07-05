#include <VL53L0X.h> 

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
    int offset[NUM_SENSORS] = {10, 0, 40, 0};

    VL53L0X sensor[NUM_SENSORS];
    
    void sensorsInit();
    void distanceRead();
    void printDistances();
    void printDistancesSensor(int sensor_num);
};



void VL53_sensors::sensorsInit() {

    //Iniciando o endereçamento dos sensores
    Wire.begin();

    for (uint8_t i = 0; i < number_sensor; i++){
      pinMode(x_shut_pins[i], OUTPUT);
      digitalWrite(x_shut_pins[i], LOW);


    }

    for (uint8_t i = 0; i < number_sensor; i++)
    {
      pinMode(x_shut_pins[i], INPUT);
      sensor[i].init(true);
      sensor[i].setAddress((uint8_t)0x21 + i); //endereço do sensor 1
      sensor[i].setTimeout(40);
    }
}

void VL53_sensors::distanceRead() {
    for (uint8_t i = 0; i < number_sensor; i++)
    {
       dist[i] = sensor[i].readRangeSingleMillimeters() - offset[i];
     
    }   
}

void VL53_sensors::printDistances() {
  for (uint8_t i = 0; i < number_sensor; i++)
  {

       Serial.print(" ");
       Serial.print(String(i));
       Serial.print(" ");
       Serial.print(dist[i]);

      Serial.println("\t\t");
  }
}

void VL53_sensors::printDistancesSensor(int sensor_num) {

    Serial.print(" ");
    Serial.print(String(sensor_num));
    Serial.print(" ");
    Serial.println(dist[sensor_num]);

}