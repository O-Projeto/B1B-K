#include <Arduino.h>
#include "MedianFilter.h"

class refletancia
{
private:
    int read_sensor; 
    int MAX_VALUE = 4095;
    int MIN_VALUE = 120; 
    int treshold = 700; 
    int pin; 
    int probability_of_border; 
   
 
    
public:
    refletancia(int PIN,float TRESHHOLD);
    void init();
    int read(); 
    bool detect_border();
};

refletancia::refletancia(int PIN, float TRESHHOLD)
{
    pin = PIN ; 
    treshold = TRESHHOLD;
}

void refletancia::init(){

    pinMode(pin,INPUT);

}

int refletancia::read(){

   read_sensor =  analogRead(pin);
   return read_sensor ;

}

bool refletancia::detect_border(){

    return read_sensor<treshold;
    
}