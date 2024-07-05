
#include "IRremote.h"

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
    pin = PIN;
}
void controle_juiz::init(){
    IrReceiver.begin(pin);

}

int controle_juiz::read(){

    if (IrReceiver.decode()) {
    IrReceiver.resume();
    button_read = IrReceiver.decodedIRData.command;
    }

    return button_read;

}
