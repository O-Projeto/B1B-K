
#include "IRremote.h"
#pragma once

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
    // se o botão foi apertado retorna o o que foi apertado
    // se não, retorna -1 (padrão??)
    if (IrReceiver.decode()) {
    IrReceiver.resume();
    button_read = IrReceiver.decodedIRData.command;
    }

    return button_read;

}
