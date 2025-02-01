#pragma once
// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
// the default pins listed are the ones used on the Redbot (ROB-12097) with
// the exception of STBY which the Redbot controls with a physical switch
#define AIN1 25
#define AIN2 33
#define BIN1 27
#define BIN2 14
#define PWMA 32
#define PWMB 13
//#define STBY 26
const int STBY=26;

#define ONE 0
#define TWO 1
#define TREE 2
#define FOUR 3
#define FIVE 4
#define SIX 5

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

// Declaração das funções
void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();
void meiaLua();
void strategy_selector();
void frenteUmPouco();

// Declaração das funções da leitura dos vl's no segundo core
void readSensorsTask(void *pvParameters);
void updateCalculatedDistance();
void printCalculatedDistance();
int calculateDistance(int distances[]);

// definição das estrategias por nome (???)
enum
{
  S0,
  S1,
  S2,
};
int strategy; // ????

// Variável global para armazenar o valor calculado
int calculatedDistance = 0;

// variaveis
float read_sensor_dir = 0;
float read_sensor_esq = 0;
bool border_dir, border_esq;
int last_line_detected = 0;
int line_detected = 0;
float start_time = 0;
float current_time = 0;
float tempoRe = 0;
bool flagRe = 0;

int read_ir = -1;
int last_ir = 0;

int vel_motor_1;
int vel_motor_2;

int mediaCentro, lastMediaCentro = 10000;

int strategyDone = 0;
int strategyStart = 0;
float start_timeStrategy = 0;
int strategyTime = 0;
int start_timeFrente = 0, frenteTime = 1000, enemyfront = 0, startFrente_flag = 0;

// search
float start_timeSearch = 0;
bool flagInit = 0;