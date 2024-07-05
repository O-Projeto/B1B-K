#pragma once
// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
// the default pins listed are the ones used on the Redbot (ROB-12097) with
// the exception of STBY which the Redbot controls with a physical switch
#define AIN1 33
#define AIN2 25
#define BIN1 14
#define BIN2 27
#define PWMA 32
#define PWMB 13
//#define STBY 26
const int STBY=26;

#define ONE 0
#define TWO 1
#define TREE 2
#define POWER 21

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 2;

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