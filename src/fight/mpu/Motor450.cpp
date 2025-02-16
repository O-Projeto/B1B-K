// Motor450.cpp
// rev 1 - May 2023 - shabaz - original code
// rev 1.1 - May 2023 - shabaz - converted to an Arduino library

#include "Motor450.h"

// *************************************************
// ********  Motor450 class implementation  ********
// *************************************************

#define DEFAULT_LEDC_FREQ 5000
#define MAX_PWM 1023

Motor450::Motor450(byte driverPin1, byte driverPin2, int channel1, int channel2, int resolution)
{
  _channel1 = channel1;
  _channel2 = channel2;
  _resolution = resolution;
  mDriverPin1 = driverPin1;
  mDriverPin2 = driverPin2;

  // Motor object constructor
  pinMode(mDriverPin1, OUTPUT);
  pinMode(mDriverPin2, OUTPUT);

  // Configuração dos canais PWM
  ledcSetup(_channel1, DEFAULT_LEDC_FREQ, _resolution);
  ledcSetup(_channel2, DEFAULT_LEDC_FREQ, _resolution);
  
  // Anexação dos pinos aos canais PWM
  ledcAttachPin(mDriverPin1, _channel1);
  ledcAttachPin(mDriverPin2, _channel2);
}

void Motor450::drive(int speed)
{
  // Limitar a velocidade dentro do intervalo permitido
  speed = constrain(speed, -1000, 1000);

  // Ajustar o PWM para o range correto (-1023 a 1023)
  int pwmValue = map(speed, -1000, 1000, -1023, 1023);

  if (speed == 0) {
    // Motor parado
    PWMWRITE(_channel1, 0);
    PWMWRITE(_channel2, 0);
  } else if (speed > 0) {
    // Motor gira para frente (canal 1)
    PWMWRITE(_channel2, 0);
    PWMWRITE(_channel1, abs(pwmValue));
  } else {
    // Motor gira para trás (canal 2)
    PWMWRITE(_channel1, 0);
    PWMWRITE(_channel2, abs(pwmValue));
  }
}

void Motor450::stop(byte brake)
{
  if (brake == 0) {
    this->drive(0); // Para o motor sem freio
  } else {
    int maxPWM = (1 << _resolution) - 1;  // Calcula 2^resolução - 1
    PWMWRITE(_channel1, 1023);
    PWMWRITE(_channel2, 1023);
  }
}

void Motor450::stop()
{
  this->drive(0); // Para o motor
}

// *************************************************
// ****   End of Motor450 class implementation  ****
// *************************************************
