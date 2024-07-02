

#ifndef CONTROLLER // include guard
#define CONTROLLER
#include <Arduino.h>

class Controller
{


public:
    float proportional();
    float integrative();
    float derivative();
    
    float setpoint_;
    float current_value_;
    
    float KP;
    float KD;
    float KI;
    
    float error;
    float last_error;
    float delta_error;
    
    float output_value;
    float integral;
    
    unsigned long time; 
    unsigned long last_time;
    float  delta_time;
    
    void debug();

    Controller(float kp, float ki, float kd);
    float output(float input_value, float current_value);

};
#endif


