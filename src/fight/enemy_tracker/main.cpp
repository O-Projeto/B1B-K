#include "config.hpp"
#include "H_bridge_TB6612.hpp"
//#include <BluetoothSerial.h>
#include "VL53_sensors.hpp"
#include <Arduino.h>
#include <Wire.h>
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"

#include "kinematics.h"
#include "controller.h"

// Descomente esta linha para ativar a depuração
// #define DEBUG_PID  

// serial print dos vls para ver o quão rápido eles reagem
#define DEBUG_VL

// PID
Controller controle_angular(1,0,0);

VL53_sensors sensores;
controle_juiz controle_sony(34);


// Handle para a fila
QueueHandle_t distanceQueue;

// Desce aabixo de 1000
refletancia qr_dir(qrDir, 800);
refletancia qr_esq(qrEsq, 800);

led_rgb LED;

Motor motor1 = Motor(AIN1, AIN2, PWMA, STBY, offsetA, 10);
Motor motor2 = Motor(BIN1, BIN2, PWMB, STBY, offsetB, 10);

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

int mediaCentro = 0;
//int lastMediaCentro = 0;
int angular = 0;

// Declaração das funções
void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();
void readSensorsTask(void *pvParameters);
void updateCalculatedDistance();
void printCalculatedDistance();
int calculateDistance(int distances[]);


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
      1,  // Prioridade da tarefa
      NULL,  // Handle da tarefa
      1  // Core
  );
}
void loop() {
  read_ir = controle_sony.read();
  if (last_ir == TWO && (read_ir == ONE || read_ir == -1)){ read_ir = TWO;}
 // if (read_ir == ONE && last_ir == ONE){last_ir = TREE; read_ir = -1;}

  read_sensor_dir = qr_dir.read();
  read_sensor_esq = qr_esq.read();
  border_dir = qr_dir.detect_border();
  border_esq = qr_esq.detect_border();
  sensores.distanceRead();

  switch (read_ir)
  {
  case ONE:
    Serial.println(read_ir);
    LED.fill(VERDE);
    start_time = millis();
    last_ir = ONE;
    break;

  case TWO:
    // // check_border();
    // // re();  
     search();
    // //totalFrente();
 
    //vel_motor_1 = 500;
    //vel_motor_2 = 500;
    drive(vel_motor_1,vel_motor_2);
    last_ir = TWO;
    break;

  case TREE:
    Serial.println(read_ir);
    drive(0,0);
    delay(10);
    LED.set(VERMELHO);
    tempoRe = 0;
    flagRe = 0;
    last_ir = TREE;
    start_time = millis();
    break;

  default:
    start_time = millis();
    LED.set(0);
    break;
}
   read_ir = controle_sony.read();


}
void drive(int mot1, int mot2){
    motor1.drive(mot1);
    motor2.drive(mot2);
}
void search()
{  // return value from to 200 to -200 -> view b1bk from front left is 200 
  if (!flagRe){

  // caso os sensores consigam 
  mediaCentro = sensores.PesosDistancias(); 
  
  if(mediaCentro != -9999){
    angular = controle_angular.output(0.0,mediaCentro);
  }


  vel_motor_1 = cinematic_left(0,angular);
  vel_motor_2 = cinematic_right(0,angular);


#ifdef DEBUG_PID
  Serial.print("enemy_pos: ");
  Serial.print(mediaCentro);
  Serial.print(" angular: ");
  Serial.print(angular);
  Serial.print(" Motor_left: ");
  Serial.print(vel_motor_1);
  Serial.print(" Motor_right: ");
  Serial.print(vel_motor_2);
  Serial.println("");
#endif

#ifdef DEBUG_VL
  sensores.printDistances();
#endif

  
}
}
// se ela ta grudada em algo ela usa força total
void totalFrente()
{
  if ((sensores.dist[1] <= 60 && sensores.dist[2]<=60) && !flagRe)
  {
    vel_motor_1 = 800;
    vel_motor_2 = 800;
  }
}

//se o tempo atual menos o tempo de inicio for menor que o tempo de ré então ele da ré 
void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe){
      if (border_dir && border_esq){
        tempoRe = 200;
        vel_motor_1 = -600;
        vel_motor_2 = -600;
        flagRe = 1;
      }else if (border_dir){
        tempoRe = 200;
        vel_motor_1 = -600;
        vel_motor_2 = -300;
        flagRe = 1;
      }else if (border_esq){
        tempoRe = 200;
        vel_motor_1 = -600;
        vel_motor_2 = -300;
        flagRe = 1;
      }else{
        vel_motor_1 = -600;
        vel_motor_2 = -600;
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

int calculateDistance(int distances[]) {

	 int Media[NUM_SENSORS] = {25,5,-5,-25}, distanciaP=0, distanciaN=0;
  for (int i=0; i<=NUM_SENSORS; i++){
    if (distances[i]>500){distances[i]=0;}
    Media[i] = distances[i]*Media[i];
  }
    distanciaP=(Media[0]+Media[1])/30;
    distanciaN=(Media[2]+Media[3])/30;
    if (distanciaP == 0 && distanciaN == 0){return -9999;}
    return (distanciaP+distanciaN);

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
    //delay(500);
}
