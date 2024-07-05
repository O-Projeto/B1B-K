#include "VL53_sensors.h"
#include <Arduino.h>
#include <Wire.h>

VL53_sensors sensores;

// Handle para a fila
QueueHandle_t distanceQueue;

// Variável global para armazenar o valor calculado
int calculatedDistance = 0;

// Declaração das funções
void readSensorsTask(void *pvParameters);
void updateCalculatedDistance();
void printCalculatedDistance();
int calculateDistance(int distances[]);


void setup() {
    Serial.begin(112500);
    sensores.sensorsInit();

    // Cria a fila
    distanceQueue = xQueueCreate(10, sizeof(int) * NUM_SENSORS);

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
    // Atualiza a variável global com a distância calculada
    updateCalculatedDistance();

    // Imprime a distância calculada armazenada na variável global
    printCalculatedDistance();

    // Continue executando outras tarefas sem delay
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

// #include "VL53_sensors.h"
// #include <Arduino.h>
// #include <Wire.h>

// VL53_sensors sensores;

// void setup() 
// {
// 	Serial.begin(112500);
// 	sensores.sensorsInit();
// }
// void loop() 
// {
// 	sensores.distanceRead();
// 	sensores.printDistances();
// }