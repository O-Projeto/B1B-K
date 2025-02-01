#include <Arduino.h>
#include "JS40F_JSumo.h"

JS40F_JSumo sensor;

void setup() 
{
	Serial.begin(115200);
	sensor.sensorsInit();
}
void loop() 
{
	sensor.distanceRead();
	sensor.printDistances();
}