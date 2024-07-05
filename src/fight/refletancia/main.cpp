#include <Arduino.h>
#include "refletancia.h"

// altere o valor 2700 de acordo com o valor de referência entre branco e preto

#define rgt_front_pin 39
#define lft_front_pin 36

float read_sensor_rgt;
float read_sensor_lft;
bool border_rgt;
bool border_lft;

refletancia rgt_front(rgt_front_pin, 2700);
refletancia lft_front(lft_front_pin, 2700);

void setup(){

    rgt_front.init();
    lft_front.init();
    Serial.begin(112500);
}



void loop(){

    read_sensor_rgt = rgt_front.read();
    read_sensor_lft = lft_front.read();

    border_rgt = rgt_front.detect_border();
    border_lft = lft_front.detect_border();

    Serial.print(read_sensor_rgt);
    Serial.print(", ");
    Serial.print(read_sensor_lft);

    Serial.print(", ");
    Serial.print(border_rgt);
    Serial.print(", ");
    Serial.print(border_lft);
    Serial.println("\t");
    delay(1000);   
}