#include <Arduino.h>
#include "refletancia.h"



#define pin 26 

refletancia rft_front(pin, 700);

void setup(){


    rft_front.init();
    Serial.begin(112500);
}



void loop(){

   float  read_sensor; 
   bool border; 
   

    read_sensor = rft_front.read();

    border = rft_front.detect_border();

    Serial.print(read_sensor);

    Serial.print("\t");

    Serial.println(border);

    
}