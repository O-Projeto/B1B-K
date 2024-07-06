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

refletancia qr_dir(qrDir, 2400);
refletancia qr_esq(qrEsq, 2400);

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

int mediaCentro, lastMediaCentro;

void drive(int mot1, int mot2);
void search();
void check_border();
void re();
void totalFrente();

// Declaração das funções
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
    updateCalculatedDistance();
    check_border();
    search();
    check_border();
    re();
    // sensores.printDistances();
    //totalFrente();
    //Serial.print(vel_motor_1,vel_motor_2);

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

  // mediaCentro = sensores.PesosDistancias();
  mediaCentro = calculatedDistance;
 // Serial.println(mediaCentro);
  if (mediaCentro == -1 && !flagRe){
    if (lastMediaCentro < -60){
      vel_motor_1 = -200;
      vel_motor_2 = 200;
    } else if (lastMediaCentro > 60){
      vel_motor_1 = 200;
      vel_motor_2 = -200;
    } else{
      vel_motor_1 = 200;
      vel_motor_2 = -200;
    }
  } else if (!flagRe){
    if (mediaCentro < -60){
      vel_motor_1 = -200;
      vel_motor_2 = 500;
    } else if (mediaCentro > 60){
      vel_motor_1 = 500;
      vel_motor_2 = -200;
    } else {
      vel_motor_1 = 500;
      vel_motor_2 = 500;
    }
    lastMediaCentro = mediaCentro;
  }
}

void totalFrente()
{
  if (sensores.dist[1] <= 60 && sensores.dist[2]<=60)
  {
    vel_motor_1 = 700;
    vel_motor_2 = 700;
  }
}

void re(){
    current_time = millis();
    if(current_time - start_time < tempoRe && flagRe){
      if (border_dir && border_esq){
        vel_motor_1 = -500;
        vel_motor_2 = -500;
      }else if (border_dir){
        vel_motor_1 = -200;
        vel_motor_2 = -500;
      }else if (border_esq){
        vel_motor_1 = -500;
        vel_motor_2 = -200;
      }else{
        vel_motor_1 = -500;
        vel_motor_2 = -500;
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

int calculateDistance(int distances[]) {
    // Exemplo de cálculo: média das distâncias
    // int sum = 0;
    // for (int i = 0; i < NUM_SENSORS; i++) {
    //     sum += distances[i];
    // }
    // return sum / NUM_SENSORS;

	 int Media[NUM_SENSORS] = {25,5,-5,-25}, distanciaP=0, distanciaN=0;
  for (int i=0; i<=NUM_SENSORS; i++){
    if (distances[i]>300){distances[i]=0;}
    Media[i] = distances[i]*Media[i];
  }
    distanciaP=(Media[0]+Media[1])/30;
    distanciaN=(Media[2]+Media[3])/30;
    if (distanciaP == 0 && distanciaN == 0){return -1;}
    return (distanciaP+distanciaN);


}