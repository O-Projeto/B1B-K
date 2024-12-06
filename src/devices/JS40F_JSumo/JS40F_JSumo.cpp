#include "JS40F_JSumo.h"

void JS40F_JSumo::sensorsInit()
{
    for (int i = 0; i< number_sensor; i++){
        pinMode(sensorPin[i], INPUT);
    }
}

void JS40F_JSumo::distanceRead()
{
    for (int i = 0; i < number_sensor; i++)
    {
        sensorRead[i] = digitalRead(sensorPin[i]);
    }
}

void JS40F_JSumo::printDistances()
{
    for (int i = 0; i< number_sensor; i++){
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" esta: ");
        Serial.print("");
        Serial.print(sensorRead[i]);
        Serial.println("");
    }
}
