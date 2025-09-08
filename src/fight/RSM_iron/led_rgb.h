//PWM control for the motors 
#include <Adafruit_NeoPixel.h>
#pragma once


#define NUMPIXELS 2
#define RGB_PIN 23

Adafruit_NeoPixel pixels(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

class led_rgb 
{
private:
  // Variáveis para controle de tempo (não-bloqueante)
    unsigned long previousMillis = 0;
    
    // Variável para controlar o estado do pisca-pisca
    bool estadoBlink = false; 
    
    // Variáveis para a função latch
    unsigned long latchStartTime = 0;
    int latchColor = 0x000000;
    bool latchAtivo = false;

public:
     // Inicializa o LED
    void init();
    
    // Define uma cor sólida no LED
    void set(int color);
    
    // Função para piscar uma única cor (liga/desliga)
    void blink(const long interval, int color);

    // Nova função para piscar duas cores de forma alternada
    void blinkAlternado(int color1, int color2);
    
    // Função para manter uma cor por um tempo e depois apagar (agora corrigida)
    void latch(const long time, int color);

    // Função para ser chamada no loop principal para atualizar o estado do latch
    void update();
};


void led_rgb ::init(){
    //set motor
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(50); // not so bright]
  pixels.fill(0x000000);
  pixels.show();

};

void led_rgb ::blink(const long time, int color){

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= time) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    estadoBlink = !estadoBlink;
           if (estadoBlink) {
            pixels.fill(color); // Liga o LED com a cor desejada
        } else {
            pixels.fill(0x000000); // Desliga o LED
        }
        pixels.show();
    }
   
  }
void led_rgb::blinkAlternado(int color1, int color2) {

        // Alterna o estado para saber qual cor mostrar
        estadoBlink = !estadoBlink;

        if (estadoBlink) {
            pixels.fill(color1); // Mostra a primeira cor
        } else {
            pixels.fill(color2); // Mostra a segunda cor
        }
        pixels.show(); // Envia a cor para o LED
}


// Função latch corrigida: apenas configura as variáveis
void led_rgb::latch(const long time, int color) {
    latchAtivo = true;
    latchStartTime = millis();
    latchColor = color;
    pixels.fill(latchColor);
    pixels.show();
}
void led_rgb::set(int color) {
    latchAtivo = false; // Cancela qualquer latch ativo
    pixels.fill(color);
    pixels.show();
}

// O update deve ser chamado no loop principal para que o latch funcione
void led_rgb::update() {
    // Se o latch estiver ativo e o tempo já passou
    if (latchAtivo && (millis() - latchStartTime > 1000)) { // 1000ms = 1s
        latchAtivo = false; // Desativa o latch
        pixels.fill(0x000000); // Apaga o LED
        pixels.show();
    }
}