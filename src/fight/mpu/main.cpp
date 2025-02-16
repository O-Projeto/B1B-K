// a main inclui uma maquina de estados que definida pelo controle roda o loop (escolha da estrategia e comandos) 

/*Verificar qual b1bk esta sendo usada para setar os trash-holds*/

// bibliotecas
#include <Arduino.h>
#include "config.hpp"
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"
#include "JS40F_JSumo.h"
#include "Motor450.h"
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>  
//#include "H_bridge_TB6612.hpp"
//#include <BluetoothSerial.h>
//#include "VL53_sensors.hpp"
//#include <Wire.h>

// inicialização dos objetos
//VL53_sensors sensores;
#define TEMPO_VIRADA 100
JS40F_JSumo sensor;


controle_juiz controle_sony(34);

refletancia qr_dir(qrDir, 3000);
refletancia qr_esq(qrEsq, 3000); // rcx era entre 50 e 100

led_rgb LED;

Motor450 motordir = Motor450(AIN1, AIN2, offsetA, offsetB,  10);
Motor450 motoresq = Motor450(BIN1, BIN2, offsetC, offsetD,  10);


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

int vel_motor_dir = 0;
int vel_motor_esq = 0;

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
int lastRead[NUM_SENSORS] = {0,0,0,0,0};

Servo myServo;  
uint8_t servoAngle = 90;  // Começa em 90°
int servoMoved = 0;  // Flag para evitar movimentos repetidos
int tempoGiro, tempoParado;

// Declaração das funções
void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();
void meiaLua();
void strategy_selector();
void frenteUmPouco();

void updateMPU();
void readMPUTask(void *parameter);
uint8_t flagMpu = 0;

// definição das estrategias por nome (???)
enum {
  S0, 
  S1,
  S2, 
};
int strategy; // ????
unsigned long inicioVirada = 0;  // Guarda o tempo inicial da virada
int virando = 0;  // Indica se está virando

Adafruit_MPU6050 mpu;
QueueHandle_t mpuQueue;

struct MPUData {
    float accZ;
    float gyroZ;
};

void setup() {
   // inicialização dos sensores, controles e led
   Serial.begin(112500);
   sensor.sensorsInit();
   controle_sony.init();
   motordir.stop(HARD_BRAKE);
   motoresq.stop(HARD_BRAKE);
   LED.init();
   LED.set(AZUL);
   delay(1000);
   LED.set(0);
   myServo.attach(SERVO);  
   myServo.write(90);  // Inicia em 90°
   for(int i = 0; i < NUM_SENSORS; i++)
   {
      lastRead[i] = 0;
   }

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

 // sensor.distanceRead();
 switch (read_ir){
 case ONE:
     last_ir = ONE;
     LED.fill(VERDE);
     start_time = millis();
     break;

 case TWO: // caso loop padrão 
   sensor.distanceRead();
   if (!servoMoved) {  // Só move se ainda não tiver se movido
    myServo.write(servoAngle);
    servoMoved = 1;  // Impede novos movimentos até atualizar o ângulo
  }
   strategy_selector();
   check_border();
   re();
   search();
   /*if (strategyDone){// se a estratégia estiver feita o código padrão volta ao normal 
   //updateCalculatedDistance();
   search();
   }*/
   drive(vel_motor_dir,vel_motor_esq); //unico lugar onde manda velocidade pros motores
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
   servoAngle = 180;
   LED.fill(BRANCO);
   last_ir = SIX;
 break;
 case SEVEN:
   servoAngle = 0;
   LED.fill(BRANCO);
   last_ir = SEVEN;
 break;
 default:
 break;
}
  read_ir = controle_sony.read();
}
void drive(int mot1, int mot2){
  if(mot1 == 0 || mot2 == 0)
  { 
    if (mot1 == 0)
    {
      motoresq.drive(mot2);
      motordir.stop(HARD_BRAKE);

    }
    if (mot2 == 0)
    {
      motoresq.stop(HARD_BRAKE);
      motordir.drive(mot1);

    }
    if (mot1 == 0 && mot2 == 0) {
      // Para ambos os motores completamente
      motoresq.stop(HARD_BRAKE);
      motordir.stop(HARD_BRAKE);
    }
  }
  else
  {
    motordir.drive(mot1);
    motoresq.drive(mot2);
  }
  
}   

void readMPUTask(void *parameter) {
  MPUData data;
  while (true) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      
      data.accZ = a.acceleration.z;
      data.gyroZ = g.gyro.z;
      
      xQueueOverwrite(mpuQueue, &data); // Sobrescreve a última leitura
      vTaskDelay(pdMS_TO_TICKS(10)); // Aguarda 10 ms
  }
}

void search()
{
  if (!flagRe){
  if (!sensor.sensorRead[0] && !sensor.sensorRead[1]  && !sensor.sensorRead[2] && !sensor.sensorRead[3] && !sensor.sensorRead[4]){
    // Se ainda não está virando, inicia a virada
    if (!virando) {
      inicioVirada = millis();  // Marca o tempo de início
      virando = 1;  // Ativa a flag de virada
      if ((lastRead[1] &&  lastRead[2] && lastRead[3]) || (lastRead[2] && !lastRead[1] && !lastRead[3] && !lastRead[4] && !lastRead[0])) {
        vel_motor_dir = 300;
        vel_motor_esq = 300;  
      }
      else if (lastRead[1] &&  lastRead[2]) {
        vel_motor_dir = 320;
        vel_motor_esq = 400;  // Vira para a direita
      }
      else if (lastRead[2] &&  lastRead[3]) {
        vel_motor_dir = 400;
        vel_motor_esq = 320;  // Vira para a direita
      }
      else if (lastRead[0]) {
        vel_motor_dir = 0;
        vel_motor_esq = 400;  
      } else if (lastRead[4]) {
        vel_motor_dir = 400;
        vel_motor_esq = 0;  // Vira para a esquerda
      } else {
        vel_motor_dir = 200;
        vel_motor_esq = 200;  // Continua reto
      }
    }

    // Se já está virando, verifica se passou 500 ms
    if (virando && millis() - inicioVirada >= TEMPO_VIRADA) 
    {
      virando = false;  // Cancela a virada
      vel_motor_dir = 400;
      vel_motor_esq = 400;  // Continua reto após a virada
    }
  }
  else
  {
    virando = 0;
    if (sensor.sensorRead[0] && sensor.sensorRead[1] && bandeira_flag == 0){
      vel_motor_dir = 300;
      vel_motor_esq = 600;
    } else if (sensor.sensorRead[0] && bandeira_flag == 0){
      vel_motor_dir = 0;
      vel_motor_esq = 500;
    }else if (sensor.sensorRead[3] && sensor.sensorRead[4] && bandeira_flag == 0){
      vel_motor_dir = 600;
      vel_motor_esq = 300;
    } else if (sensor.sensorRead[4] && bandeira_flag == 0){
      vel_motor_dir = 500;
      vel_motor_esq = 0;
    }
    else if (sensor.sensorRead[1] && sensor.sensorRead[2] && sensor.sensorRead[3] && bandeira_flag == 0){
      vel_motor_dir = 1000;
      vel_motor_esq = 1000;
    }
    else if (sensor.sensorRead[1] && sensor.sensorRead[2] && bandeira_flag == 0){
      vel_motor_dir = 1000;
      vel_motor_esq = 1000;
    }else if (sensor.sensorRead[2] && sensor.sensorRead[3] && bandeira_flag == 0){
      vel_motor_dir = 1000;
      vel_motor_esq = 1000;
    }
    else if (sensor.sensorRead[2] && bandeira_flag == 0){
      vel_motor_dir = 800;
      vel_motor_esq = 800;
    } 
        // loop att leastread
    for (int i=0; i< NUM_SENSORS; i++){
      lastRead[i]= sensor.sensorRead[i];
    }
  }
  }
  else
  {
    if (sensor.sensorRead[0] && bandeira_flag == 0){
      vel_motor_dir = 0;
      vel_motor_esq = 500;
      for (int i=0; i< NUM_SENSORS; i++){
        lastRead[i]= sensor.sensorRead[i];
      }
    }
    else if (sensor.sensorRead[4] && bandeira_flag == 0){
      vel_motor_dir = 500;
      vel_motor_esq = 0;
      for (int i=0; i< NUM_SENSORS; i++){
        lastRead[i]= sensor.sensorRead[i];
      }
    }
    else if (sensor.sensorRead[1] && sensor.sensorRead[2] && sensor.sensorRead[3] && bandeira_flag == 0){
      vel_motor_dir = 1000;
      vel_motor_esq = 1000;
      for (int i=0; i< NUM_SENSORS; i++){
        lastRead[i]= sensor.sensorRead[i];
      }
    }
    else if (sensor.sensorRead[2] && bandeira_flag == 0){
      vel_motor_dir = 700;
      vel_motor_esq = 700;
      for (int i=0; i< NUM_SENSORS; i++){
        lastRead[i]= sensor.sensorRead[i];
      }
    } 
  }
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
  else {line_detected = 0;}
  }
 
void updateMPU()
{
  MPUData data;
  if (xQueueReceive(mpuQueue, &data, 0)) {
      Serial.print(data.accZ);
      Serial.println(data.gyroZ);
  }
  vTaskDelay(pdMS_TO_TICKS(10)); 
}
  
void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe && flagRe){
      // se o tempo de ré não tiver passado ele ajeita o robo e volta reto
      if (border_dir && border_esq){
        tempoRe = 200;
        vel_motor_dir = -500;
        vel_motor_esq = -500;
      }else if (border_dir){
        tempoRe = 300;
        vel_motor_dir = -700;
        vel_motor_esq = -250;
      }else if (border_esq){
        tempoRe = 300;
        vel_motor_dir = -250;
        vel_motor_esq = -500;
      }else
      {
        vel_motor_dir = -400;
        vel_motor_esq = -400;
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
    vel_motor_dir = 400;
    vel_motor_esq = 300;
    } else {
    strategyDone = 1;
    }
  }


void frenteUmPouco()
{ // vai pra frente por um tempo estimulado
  current_time = millis();
  if (current_time - start_timeStrategy <= strategyTime){
  vel_motor_dir = 400;
  vel_motor_esq = 400;
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
    vel_motor_dir = 1000;
    vel_motor_esq = 1000;
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
      strategyTime = 200;
      meiaLua();
      break;   
    case S1:
      strategyTime = 150;
      frenteUmPouco();
      break;
    default:
      strategyDone = 1;
      break;
    }
    
  }
} // 
