// a main inclui uma maquina de estados que definida pelo controle roda o loop (escolha da estrategia e comandos) 

/*Verificar qual b1bk esta sendo usada para setar os trash-holds*/

// bibliotecas
#include <Arduino.h>
#include "config.hpp"
//#include "H_bridge_TB6612.hpp"
//#include <BluetoothSerial.h>
//#include "VL53_sensors.hpp"
//#include <Wire.h>
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"
#include "JS40F_JSumo.h"
#include "Motor450.h"

// inicialização dos objetos
//VL53_sensors sensores;
JS40F_JSumo sensor;

// Handle para a fila
QueueHandle_t distanceQueue;

controle_juiz controle_sony(34);

refletancia qr_dir(qrDir, 50);
refletancia qr_esq(qrEsq, 50); // rcx era entre 50 e 100

led_rgb LED;

Motor450 motor1 = Motor450(AIN1, AIN2, offsetA, offsetB,  10);
Motor450 motor2 = Motor450(BIN1, BIN2, offsetC, offsetD,  10);

// Variável global para armazenar o valor calculado
int calculatedDistance = 0;

// variaveis 
float  read_sensor_dir = 0;
float read_sensor_esq = 0; 
bool border_dir, border_esq; 
int last_line_detected = 0;
int line_detected = 0;
float start_time = 0;
float current_time = 0;
float tempoRe=0;
bool flagRe= 0;
bool prioridadeAtaque = 0;
bool bandeira_flag = 0;

int read_ir = -1;
int last_ir = 0;

int vel_motor_1;
int vel_motor_2;

int mediaCentro, lastMediaCentro=10000;

int strategyDone=0;
int strategyStart=0;
float start_timeStrategy = 0;
int strategyTime = 0;
int start_timeFrente=0,frenteTime=1000, enemyfront= 0, startFrente_flag = 0; 

// search
float start_timeSearch = 0;
bool flagInit = 0;
int sensorRead[NUM_SENSORS];

// Declaração das funções
void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();
void meiaLua();
void strategy_selector();
void frenteUmPouco();

void updateCalculatedDistance();
void readSensorsTask(void *pvParameters);

// definição das estrategias por nome (???)
enum {
  S0, 
  S1,
  S2, 
};
int strategy; // ????

// #define VL_SENSOR
#define JSUMO

void setup() {
 
   // Cria a fila
   distanceQueue = xQueueCreate(10, sizeof(int) * NUM_SENSORS);

   // inicialização dos sensores, controles e led
   Serial.begin(112500);
   sensor.sensorsInit();
   controle_sony.init();
   LED.init();
   LED.set(AZUL);
   delay(1000);
   LED.set(0);
 
   // Cria a tarefa no Core 1 para ler as distâncias dos sensores
   xTaskCreatePinnedToCore(
       readSensorsTask,  // Função da tarefa
       "ReadSensorsTask",  // Nome da tarefa
       2048,  // Tamanho da pilha
       NULL,  // Parâmetro da tarefa
       2,  // Prioridade da tarefa
       NULL,  // Handle da tarefa
       0  // Core
   );
}

void loop() {
 // leitura do controle e filtro 
 read_ir = controle_sony.read();
 if (last_ir == TWO && (read_ir != TREE)){read_ir = TWO;}
 else if (last_ir == TREE){read_ir = TREE;}

 // leitura qr e teste de borda
 read_sensor_dir = qr_dir.read();
 read_sensor_esq = qr_esq.read();
 border_dir = qr_dir.detect_border();
 border_esq = qr_esq.detect_border();

 switch (read_ir){
 case ONE:
     last_ir = ONE;
     LED.fill(VERDE);
     start_time = millis();
     break;

 case TWO: // caso loop padrão 
   // sensores.distanceRead();
   strategy_selector();
   check_border();
   re();
   if (strategyDone){// se a estratégia estiver feita o código padrão volta ao normal 
   //updateCalculatedDistance();
   search();
   }
   drive(vel_motor_1,vel_motor_2); //unico lugar onde manda velocidade pros motores
   last_ir = TWO;
   break;
 case TREE: //reinicializa tudo (aparentemente não pode por regra do sumo)
   drive(0,0);
   delay(10);
   LED.set(VERMELHO);
   tempoRe = 0;
   flagRe = 0;
   last_ir = TREE;
   strategyStart = 0;
   strategyDone=0;
   lastMediaCentro = 0;
   enemyfront = 0;
   break;
 case FOUR:
   strategy = S0;
   last_ir = FOUR;
   LED.fill(MAGENTA);
 break;
 case FIVE:
   strategy = S1;
   last_ir = FIVE;
   LED.fill(MAGENTA);
 break;
 case SIX:
   prioridadeAtaque = 1;
   LED.fill(BRANCO);
   last_ir = SIX;
 break;
 default:
 break;
}
  read_ir = controle_sony.read();
}
void drive(int mot1, int mot2){
   motor1.drive(mot1);
   motor2.drive(mot2);
}  

void check_border()
{
  if (line_detected != last_line_detected) // testa pra caso esteja realmente vendo a linha
  {
    start_time = millis();
  }
    last_line_detected = line_detected; 
  if (border_dir || border_esq){ // se viu qualquer borda inicializa as variaveis pra entrar na preferencia da ré e ter 
  // "tempo" de arrumar
    tempoRe = 200;
    line_detected = 1;
    flagRe = 1;
  }
  else {line_detected=0;}
  }
 
void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe && flagRe){
      // se o tempo de ré não tiver passado ele ajeita o robo e volta reto
      if (border_dir && border_esq){
        tempoRe = 300;
        vel_motor_1 = -700;
        vel_motor_2 = -700;
      }else if (border_dir){
        tempoRe = 300;
        vel_motor_1 = -200;
        vel_motor_2 = -800;
      }else if (border_esq){
        tempoRe = 300;
        vel_motor_1 = -800;
        vel_motor_2 = -200;
      }else{
        vel_motor_1 = -700;
        vel_motor_2 = -700;
      }
    } else{ // zera as variaveis  
        tempoRe = 0;
        flagRe = 0;
    }
}
  
void meiaLua()
  { //estratégia que gira em meia lua por certo tempo
    current_time = millis();
    if (current_time - start_timeStrategy <= strategyTime){
    vel_motor_1 = 1000;
    vel_motor_2 = 600;
    } else {
    strategyDone = 1;
    }
  }
  
void frenteUmPouco()
{ // vai pra frente por um tempo estimulado
  current_time = millis();
  if (current_time - start_timeStrategy <= strategyTime){
  vel_motor_1 = 400;
  vel_motor_2 = 400;
  } else {
  strategyDone = 1;
  }

}

void totalFrente()
{ 
  if(!startFrente_flag)
    start_timeFrente = millis ();
  startFrente_flag = 1;
  if (millis() - start_timeFrente >= frenteTime){
    vel_motor_1 = 1000;
    vel_motor_2 = 1000;
  }
}

void strategy_selector()
{
  if (!strategyStart){
      start_timeStrategy = millis();
      strategyStart = 1;
      }
  if (!strategyDone){
    switch (strategy){
    case S0:
      
      break;   
    case S1:
      strategyTime = 100;
      frenteUmPouco();
      break;
    }
  }
} // 
void readSensorsTask(void *pvParameters) {
  int distances[NUM_SENSORS];
  while (1) {
      sensor.distanceRead();
      for (int i = 0; i < NUM_SENSORS; i++) {
        distances[i] = sensor.sensorRead[i];
      // Envia as distâncias para a fila sem bloquear
      xQueueSendFromISR(distanceQueue, &distances, NULL);
  }
}
}

void updateCalculatedDistance() {
  int distances[NUM_SENSORS];
  // Tenta ler da fila sem bloquear
  if (xQueueReceive(distanceQueue, &distances, 0)) {
      // Calcula o valor com base nas leituras dos sensores
    //  calculatedDistance = calculateDistance(distances);
    for (int i = 0; i < NUM_SENSORS; i++) {
       sensorRead[i] = distances[i] ;
}
  }
}

#ifdef VL_SENSOR
#include "VL53_sensors.h"
// Objeto VLs
VL53_sensors sensores;
void VL_attack()
{
  if(sensores.dist[1] <= 50 && sensores.dist[2] <= 50 && bandeira_flag == 0)
  {
    vel_motor_1 = 950;
    vel_motor_2 = 950;
  }
  else if(sensores.dist[1] <= 100 && sensores.dist[2] <= 100 && bandeira_flag == 0)
  {
    vel_motor_1 = 700;
    vel_motor_2 = 700;
  }
  else if(sensores.dist[1] <= 200 && sensores.dist[0] <= 100 && bandeira_flag == 0)
  {
    vel_motor_1 = 700;
    vel_motor_2 = 450;
  }
  else if(sensores.dist[2] <= 200 && sensores.dist[3] <= 100 && bandeira_flag == 0)
  {
    vel_motor_1 = 450;
    vel_motor_2 = 700;
  }
  else if (bandeira_flag)
  {
    if (sensores.dist[1] <= 50 && sensores.dist[2] <= 50 && sensores.dist[3] <= 80 && sensores.dist[0] <= 80)
    {
      vel_motor_1 = 950;
      vel_motor_2 = 950;
    }
    else if(sensores.dist[1] <= 100 && sensores.dist[2] <= 100 && sensores.dist[3] <= 200 && sensores.dist[0] <= 200)
    {
      vel_motor_1 = 700;
      vel_motor_2 = 700;
    }
  }
}
#endif

#ifdef JSUMO
#include "JS40F_JSumo.h"
JS40F_JSumo sensores;
void search()
{
  if (sensor.sensorRead[0] && bandeira_flag == 0){
    vel_motor_1 = 700;
    vel_motor_2 = 300;
  } else if(sensor.sensorRead[0]&& sensor.sensorRead[1] && bandeira_flag == 0){
    vel_motor_1 = 500;
    vel_motor_2 = 200;
  } else if (sensor.sensorRead[1]  && sensor.sensorRead[2] && bandeira_flag == 0){
    vel_motor_1 = 500;
    vel_motor_2 = 200;
  } else if (sensor.sensorRead[2] && bandeira_flag == 0){
    vel_motor_1 = 800;
    vel_motor_2 = 800;
  }else if(sensor.sensorRead[1]  && sensor.sensorRead[2] && sensor.sensorRead[3] && bandeira_flag == 0){
    vel_motor_1 = 1000;
    vel_motor_2 = 1000;
    totalFrente();
  } else if(sensor.sensorRead[2] && sensor.sensorRead[3] && bandeira_flag == 0){
    vel_motor_1 = 200;
    vel_motor_2 = 500;
  } else if(sensor.sensorRead[3] && sensor.sensorRead[4] && bandeira_flag == 0){
    vel_motor_1 = 200;
    vel_motor_2 = 500;
  } else if (sensor.sensorRead[4] && bandeira_flag == 0){
    vel_motor_1 = 300;
    vel_motor_2 = 700;
  }
}
#endif