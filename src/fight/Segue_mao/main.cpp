

/*Verificar qual b1bk esta sendo usada para setar os trash-holds*/


#include "config.hpp"
#include "H_bridge_TB6612.hpp"
//#include <BluetoothSerial.h>
#include "VL53_sensors.hpp"
#include <Arduino.h>
#include <Wire.h>
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"

VL53_sensors sensores;

// Handle para a fila
QueueHandle_t distanceQueue;

controle_juiz controle_sony(34);

refletancia qr_dir(qrDir, 800);
refletancia qr_esq(qrEsq, 800);

led_rgb LED;

Motor motor1 = Motor(AIN1, AIN2, PWMA, STBY, offsetA, 10);
Motor motor2 = Motor(BIN1, BIN2, PWMB, STBY, offsetB, 10);

// Variável global para armazenar o valor calculado
int calculatedDistance = 0;

float  read_sensor_dir = 0;
float read_sensor_esq = 0; 
bool border_dir, border_esq; 
int last_line_detected = 0;
int line_detected = 0;
float start_time = 0;
float current_time = 0;
float tempoRe=0;
bool flagRe= 0;

int read_ir = -1;
int last_ir = 0;

int vel_motor_1;
int vel_motor_2;

int mediaCentro, lastMediaCentro=0;

int strategyDone=0;
int strategyStart=0;
float start_timeStrategy = 0;
int strategyTime = 0;
int start_timeFrente=0,frenteTime=1000, enemyfront= 0; 


void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();
void meiaLua();
void strategy_selector();
void frenteUmPouco();

// Declaração das funções
void readSensorsTask(void *pvParameters);
void updateCalculatedDistance();
void printCalculatedDistance();
int calculateDistance(int distances[]);



enum {
  S0, 
  S1,
  S2, 
};
int strategy;

void setup() 
{
   // Cria a fila
  distanceQueue = xQueueCreate(10, sizeof(int) * NUM_SENSORS);

	Serial.begin(112500);
	sensores.sensorsInit();
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
  read_ir = controle_sony.read();
  if (last_ir == TWO && (read_ir == ONE || read_ir == -1)){ read_ir = TWO;}


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

  case TWO:
    // sensores.distanceRead();
    strategy_selector();
    if (strategyDone){
    updateCalculatedDistance();
    search();
    //printCalculatedDistance();
    //sensores.printDistances();
    /*
    Serial.print("Velocidade Direita: ");
    Serial.print(vel_motor_2);
    Serial.print("   Velocidade Esquerda: ");
    Serial.println(vel_motor_1);*/
    //totalFrente();
    }
    check_border();
    re();
    drive(vel_motor_1,vel_motor_2);
    last_ir = TWO;
    break;
  case TREE:
    drive(0,0);
    delay(10);
    LED.set(VERMELHO);
    tempoRe = 0;
    flagRe = 0;
    last_ir = TREE;
    strategyStart = 0;
    strategyDone=0;
    lastMediaCentro = 0;
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
    strategy = S2;
    last_ir = S2;
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
void search()
{

    // Atualiza a variável global com a distância calculada
    

    // Imprime a distância calculada armazenada na variável global
    // printCalculatedDistance();
if (!flagRe){
  // mediaCentro = sensores.PesosDistancias();
  mediaCentro = calculatedDistance;
 // Serial.println(mediaCentro);
  if (mediaCentro == -9999){
    if (lastMediaCentro < -60){
      vel_motor_1 = 0;
      vel_motor_2 = 0;
    } else if (lastMediaCentro > 60){
      vel_motor_1 =0;
      vel_motor_2 = 0;
    } else{
      vel_motor_1 = 0;
      vel_motor_2 = 0;
    }
    }else if(mediaCentro < 150 && mediaCentro > 60){ 
      vel_motor_1 = 600;
      vel_motor_2 = 200;
    }else if (mediaCentro > -150 && mediaCentro < -60){
      vel_motor_1 = 200;
      vel_motor_2 = 600;
    }else if (mediaCentro < -150){
      vel_motor_1 = 200;
      vel_motor_2 = 400;
    } else if (mediaCentro > 150){
      vel_motor_1 = 400;
      vel_motor_2 = 200;
    } else {
      vel_motor_1 = 0;
      vel_motor_2 = 0;
    }
    lastMediaCentro = mediaCentro;
  }
}

void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe && flagRe){
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
    } else{        
        tempoRe = 0;
        flagRe = 0;
    }
}

void check_border()
{
  if (line_detected != last_line_detected)
  {
    start_time = millis();
  }
    last_line_detected = line_detected;
  if (border_dir || border_esq){
    tempoRe = 200;
    line_detected = 1;
    flagRe = 1;
  }
  else {line_detected=0;}
  }



int calculateDistance(int distances[]) {
	 int Media[NUM_SENSORS] = {25,5,-5,-25}, distanciaP=0, distanciaN=0;
  for (int i=0; i<=NUM_SENSORS; i++){
    if (distances[i]>300){distances[i]=0;}
    Media[i] = distances[i]*Media[i];
  }
    distanciaP=(Media[0]+Media[1])/30;
    distanciaN=(Media[2]+Media[3])/30;
    if (distanciaP == 0 && distanciaN == 0){return -9999;}
    return (distanciaP+distanciaN);


}

void meiaLua()
{
  current_time = millis();
  if (current_time - start_timeStrategy <= strategyTime){
  vel_motor_1 = 1000;
  vel_motor_2 = 600;
  } else {
  strategyDone = 1;
  }

}

void frenteUmPouco()
{
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
  if (!flagRe){
    if ((sensores.dist[1] <= 80 && sensores.dist[2]<=80) && !enemyfront){

      start_time = millis ();
      enemyfront = 1;
    } else if (!(sensores.dist[1] <= 80 && sensores.dist[2]<=80)){
      enemyfront = 0;
    }
  
  if (millis() - start_time >= frenteTime){
  vel_motor_1 = 1000;
  vel_motor_2 = 1000;
  } 
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
      strategyTime = 500;
      frenteUmPouco();
      break;   
    case S1:
      strategyTime = 3000;
      meiaLua();
      break;
      case S2:
      strategyTime = 60000;
      meiaLua();
      break;
    }
  }
}
void updateCalculatedDistance() {
    int distances[NUM_SENSORS];

    // Tenta ler da fila sem bloquear
    if (xQueueReceive(distanceQueue, &distances, 0)) {
        // Calcula o valor com base nas leituras dos sensores
        calculatedDistance = calculateDistance(distances);
    }
}

void printCalculatedDistance() {
    Serial.print("Calculated Distance: ");
    Serial.println(calculatedDistance);
}
void readSensorsTask(void *pvParameters) {
    int distances[NUM_SENSORS];
    while (1) {
        sensores.distanceRead();
        for (int i = 0; i < NUM_SENSORS; i++) {
            distances[i] = sensores.dist[i];
        }
        // Envia as distâncias para a fila sem bloquear
        xQueueSendFromISR(distanceQueue, &distances, NULL);
    }
}
