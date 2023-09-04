//PWM control for the motors 
#include <Adafruit_NeoPixel.h>



#define NUMPIXELS 7
#define RGB_PIN 4

Adafruit_NeoPixel pixels(NUMPIXELS, 4, NEO_GRB + NEO_KHZ800);

class led_rgb 
{
private:
    unsigned long previousMillis = 0; 
    const long interval = 1000;
    int ledState = LOW;
    unsigned long start_time ;

  
public:

    void init();
    void blink(const long time, int color);

    void latch(const long time, int color);
};


void led_rgb ::init(){
    //set motor
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(100); // not so bright]
  pixels.fill(0x000000);
  pixels.show();
  unsigned long start_time = millis();

};

void led_rgb ::blink(const long time, int color){

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= time) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    if(ledState){
         pixels.fill(color);
         pixels.show();
    }else{
        pixels.fill(0x000000);
        pixels.show();
    }
   
  }
   
}

void led_rgb ::latch(const long time, int color){

  unsigned long currentMillis = millis();

  if (currentMillis - start_time <= time) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // set the LED with the ledState of the variable:
   
    pixels.fill(color);
    pixels.show();
    
   
  }else{
        start_time = millis();
        pixels.fill(0x000000);
        pixels.show();
  }
   
}