#include "config.h"
#include <Arduino.h>

#define SATURATION 1000
#define L 1
#define R 1 

int speed_converter(int speed){

    if(speed>SATURATION){
      speed = SATURATION ;
    }
    if(speed < -SATURATION){
      speed = -SATURATION ;
    }

    return speed ; 
}
// range from -1000 to 1000 in linear and angular 
float cinematic_right(float linear, float angular){

  

  float right_speed = (linear + angular*L)/R ; 

   right_speed = speed_converter(right_speed);

  return right_speed; 

}

// range from -1000 to 1000 in linear and angular 
float cinematic_left(float linear, float  angular){

  float left_speed = (linear - angular*L)/R ;

  left_speed = speed_converter(left_speed);

  return left_speed; 

}