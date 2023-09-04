#include "VL53_sensors.h"
#include <Arduino.h>
#include <Wire.h>

VL53_sensors sensores;

void setup() 
{
	Serial.begin(112500);
	sensores.sensorsInit();
}
void loop() 
{
	sensores.distanceRead();
	sensores.printDistances();
}