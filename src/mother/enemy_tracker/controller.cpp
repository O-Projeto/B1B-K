#include "controller.h"


Controller::Controller(float kp,float ki, float kd)
{   KP = kp;
    KD = kd;
    KI = ki;
    integral = 0;
    time = 0 ; 
    last_time = 0 ;
    delta_time= 0 ;
    error = 0;
    last_error = 0;
}

float Controller::output(float setpoint, float current_value){
    setpoint_ = setpoint;
    current_value_ = current_value;
    last_error = error;
    error =  setpoint_ - current_value_ ;
    time = millis();
    delta_time = (double)(time - last_time)/10000;
    last_time = time;
    // if(setpoint_ == 0){
    //     output_value =  current_value_ + proportional() + derivative() ;
    // }else{
    // }

    output_value =  proportional() + integrative() + derivative();
    // output_value = saturation(output_value,1000);
  
    return output_value;
}

float Controller::proportional(){

    return error*KP; 

}

float Controller::integrative(){
    integral += error*delta_time;
    // integral = saturation(integral,1000);
    return integral*KI;

}

float Controller::derivative(){
   
    if (last_error - error!= 0){
        delta_error = (last_error - error)/delta_time;
    }
    else {
        delta_error = 0; 
    }
    
    return delta_error*KD;

}

void Controller::debug(){
    
    // Serial.print(" |delta_time: ");
    // Serial.print(delta_time);

    Serial.print("|input_values: ");
    Serial.print(setpoint_);
    Serial.print("|current_values: ");
    Serial.print(current_value_);

    Serial.print("||error: ");
    Serial.print(error);
    Serial.print("|P: ");
    Serial.print(proportional());
    Serial.print("|I: ");
    Serial.print(integrative());
    Serial.print("|D: ");
    Serial.print(derivative());

    Serial.print("|output_value: ");
    Serial.print(output_value);
    // Serial.println("");

}




