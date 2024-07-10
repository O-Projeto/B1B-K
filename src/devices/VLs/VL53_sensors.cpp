#include "VL53_sensors.h"


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
       Serial.print(",  ");
      //Serial.println("\t\t");
      
  }
  Serial.println("\t\t");
  // delay(1000);
}

void VL53_sensors::printDistancesSensor(int sensor_num) {

    Serial.print(" ");
    Serial.print(String(sensor_num));
    Serial.print(" ");
    Serial.println(dist[sensor_num]);

}

int VL53_sensors::PesosDistancias() {
   
  int pesos = 10, Media[number_sensor], distancia=0;
  for (int i=0; i<=number_sensor; i++){
    Media[i] = dist[i]*pesos;
    pesos = pesos * 10;
    distancia+=Media[i];
  }
    return distancia/11110;
}
