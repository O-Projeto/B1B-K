#include <Arduino.h>
#include "MedianFilter.h"
#pragma once

class refletancia
{
private:
    int read_sensor; 
    int MAX_VALUE = 4095;
    int MIN_VALUE = 120; 
    int treshold = 1500; 
    int pin; 
    int probability_of_border; 
   
 
    
public:
    refletancia(int PIN,float TRESHHOLD);
    void init();
    void read(); 
    bool detect_border();
    void printborder();
};

refletancia::refletancia(int PIN, float TRESHHOLD)
{
    pin = PIN ; 
    treshold = TRESHHOLD;
}

void refletancia::init(){

    pinMode(pin,INPUT);

}

void refletancia::read(){

   read_sensor =  analogRead(pin);

}
void refletancia::printborder(){

     Serial.print(read_sensor);
}

bool refletancia::detect_border(){
    read();
    return read_sensor<treshold;
    
}