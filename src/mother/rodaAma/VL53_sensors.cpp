#include "VL53_sensors.hpp"

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
  delay(1000);
}

void VL53_sensors::printDistancesSensor(int sensor_num) {

    Serial.print(" ");
    Serial.print(String(sensor_num));
    Serial.print(" ");
    Serial.println(dist[sensor_num]);

}
// 
/*testa todos as distancias, filtrando as que tão muito longe 
  multiplica as distancias pelos pesos 
  soma a distancia positiva e faz a media (msm coisa ngativa)
  testa se os dois tao zrados e se nao soma e retorna o valores*/
int VL53_sensors::PesosDistancias()
{ 
  int Media[number_sensor] = {25,5,-5,-25}, distanciaP=0, distanciaN=0;
  for (int i=0; i<=number_sensor; i++){
    if (dist[i]>300){dist[i]=0;}
    Media[i] = dist[i]*Media[i];
  }
    distanciaP=(Media[0]+Media[1])/30;
    distanciaN=(Media[2]+Media[3])/30;
    if (distanciaP == 0 && distanciaN == 0){return -1;}
    return (distanciaP+distanciaN);
}
