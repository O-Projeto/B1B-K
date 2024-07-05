
#include "IRremote.h"
#pragma once

#define INIT 1
int last_button_read;

class controle_juiz
{
private:
    int pin;
    int button_read = -1;
    /* data */
public:
    controle_juiz(int PIN);
    void init();
    int read();
   
};

controle_juiz::controle_juiz(int PIN)
{
    // construtora que define o pin 
    pin = PIN;
}

void controle_juiz::init(){
    // inicializa o controle
    IrReceiver.begin(pin);

}

int controle_juiz::read(){

    if (IrReceiver.decode()){
        IrReceiver.resume();
        button_read = IrReceiver.decodedIRData.command;
        if (last_button_read == 0){
            button_read = -1;
        }
    }

    last_button_read = button_read;
    return button_read;
}
