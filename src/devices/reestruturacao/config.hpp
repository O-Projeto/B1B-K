#pragma once
// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
// the default pins listed are the ones used on the Redbot (ROB-12097) with
// the exception of STBY which the Redbot controls with a physical switch
#define AIN1 25
#define AIN2 33
#define BIN1 27
#define BIN2 14
#define SERVO 13

#define ONE 0
#define TWO 1
#define TREE 2
#define FOUR 3
#define FIVE 4
#define SIX 5
#define SEVEN 6

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 2;
const int offsetC = 3;
const int offsetD = 4;

#define qrDir 39
#define qrEsq 36

//  Definição das cores do led rgb
#define AMARELO 16768256 
#define VERMELHO 16515843
#define VERDE 63240
#define AZUL 49911
#define MAGENTA 16711935
#define CIANO 65535
#define BRANCO 16777215
#define LARANJA 16542467

#define SENSOR_LATERAL_ESQ 0
#define SENSOR_FRENTE_ESQ  1
#define SENSOR_FRENTE_CTR  2
#define SENSOR_FRENTE_DIR  3
#define SENSOR_LATERAL_DIR 4
#define NUM_SENSORES 5