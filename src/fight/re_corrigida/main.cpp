// a main inclui uma maquina de estados que definida pelo controle roda o loop (escolha da estrategia e comandos)

/*Verificar qual b1bk esta sendo usada para setar os trash-holds*/

// bibliotecas
#include "config.hpp"
#include "H_bridge_TB6612.hpp"
// #include <BluetoothSerial.h>
#include "VL53_sensors.hpp"
#include <Arduino.h>
#include <Wire.h>
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"

// inicialização dos objetos
VL53_sensors sensores;

// Handle para a fila
QueueHandle_t distanceQueue;

controle_juiz controle_sony(34);

refletancia qr_dir(qrDir, 1000);
refletancia qr_esq(qrEsq, 1000); // rcx era entre 50 e 100

led_rgb LED;

Motor motor1 = Motor(AIN1, AIN2, PWMA, STBY, offsetA, 10);
Motor motor2 = Motor(BIN1, BIN2, PWMB, STBY, offsetB, 10);



void setup()
{
  // Cria a fila
  distanceQueue = xQueueCreate(10, sizeof(int) * NUM_SENSORS);

  // inicialização dos sensores, controles e led
  Serial.begin(112500);
  sensores.sensorsInit();
  controle_sony.init();
  LED.init();
  LED.set(AZUL);
  delay(1000);
  LED.set(0);

  // Cria a tarefa no Core 1 para ler as distâncias dos sensores
  xTaskCreatePinnedToCore(
      readSensorsTask,   // Função da tarefa
      "ReadSensorsTask", // Nome da tarefa
      2048,              // Tamanho da pilha
      NULL,              // Parâmetro da tarefa
      2,                 // Prioridade da tarefa
      NULL,              // Handle da tarefa
      0                  // Core
  );
}
void loop()
{
  // leitura do controle e filtro
  read_ir = controle_sony.read();
  if (last_ir == TWO && (read_ir == ONE || read_ir == -1))
  {
    read_ir = TWO;
  }

  // leitura qr e teste de borda
  read_sensor_dir = qr_dir.read();
  read_sensor_esq = qr_esq.read();
  border_dir = qr_dir.detect_border();
  border_esq = qr_esq.detect_border();

  switch (read_ir)
  {
  case ONE:
    last_ir = ONE;
    LED.fill(VERDE);
    start_time = millis(); //  REVIEW: Isso não pode dar conflito em algum momento se por acaso ele receber o comando 1?, linha 63 impede isso? Precisa dele aqui?
    break;

  case TWO: // caso loop padrão
    // sensores.distanceRead();
    strategy_selector();
    check_border();
    re();
    if (strategyDone)
    { // se a estratégia estiver feita o código padrão volta ao normal
      updateCalculatedDistance();
      search();
    }
    drive(vel_motor_1, vel_motor_2); // unico lugar onde manda velocidade pros motores
    last_ir = TWO;
    break;
  case TREE: // reinicializa tudo (aparentemente não pode por regra do sumo)
    drive(0, 0);
    delay(10);
    LED.set(VERMELHO);
    tempoRe = 0;
    flagRe = 0;
    last_ir = TREE;
    strategyStart = 0;
    strategyDone = 0;
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
    strategy = S2;
    last_ir = S2;
    break;
  default:
    break;
  }
  read_ir = controle_sony.read();
}
void drive(int mot1, int mot2)
{
  motor1.drive(mot1);
  motor2.drive(mot2);
}
void search()
{
  // Atualiza a variável global com a distância calculada
  // Imprime a distância calculada armazenada na variável global
  // printCalculatedDistance();
  if (!flagInit)
    start_timeSearch = millis();
  if (!flagRe)
  { // prioridade da ré
    mediaCentro = calculatedDistance;
    if (mediaCentro == -9999)
    { 
      // se perdeu o adversario
      enemyfront = 0;
      // melhoria de quadrantes vai ser implementada aqui
      flagInit = 1;
      if (millis() - start_timeSearch < 200)
      {
        if (lastMediaCentro < 0 && lastMediaCentro > -9999)
        { // usando a memoria identifica o ultimo lado que algo foi visto
          vel_motor_1 = -300;
          vel_motor_2 = 300;
        }
        else
        {
          vel_motor_1 = 300;
          vel_motor_2 = -300;
        }
      }
      else if (millis() - start_timeSearch < 600)
      {
        vel_motor_1 = 0;
        vel_motor_2 = 0;
      }
      else
      {
        flagInit = 0;
      }
    }
    else
    {
      flagInit = 0;
      // aqui o search filtra para qual direção
      // 1 - esquerda longe - lento esquerda
      if (mediaCentro <= -200)
      {
        vel_motor_1 = 100;
        vel_motor_2 = 250;
        enemyfront = 0; // tem que rever onde essa variavel ta sendo zerada
        // 2 - esquerda perto - rápido esquerda
      }
      else if (mediaCentro > -200 && mediaCentro < -51)
      {
        vel_motor_1 = 300;
        vel_motor_2 = 600;
        enemyfront = 0;
        // 3 - frente esquerda longe - lento esquerda
      }
      else if (mediaCentro > -50 && mediaCentro < -26)
      {
        vel_motor_1 = 100;
        vel_motor_2 = 250;
        enemyfront = 0;
        // 4 - frente esquerda perto - rápido esquerda
      }
      else if (mediaCentro > -25 && mediaCentro < -11)
      {
        vel_motor_1 = 300;
        vel_motor_2 = 600;
        enemyfront = 0;
        // 5 - frente !!! - Ataca!!!
      }
      else if (mediaCentro <= 10 && mediaCentro >= -10)
      {
        vel_motor_1 = 600;
        vel_motor_2 = 600;
        enemyfront = 1;
        // totalFrente();
        // 6 - frente direita perto - rápido direita
      }
      else if (mediaCentro <= 25 && mediaCentro > 11)
      {
        vel_motor_1 = 600;
        vel_motor_2 = 300;
        enemyfront = 0;
        // 7 - frente direita longe - lento direita
      }
      else if (mediaCentro <= 50 && mediaCentro > 26)
      {
        enemyfront = 0;
        vel_motor_1 = 250;
        vel_motor_2 = 100;
        // 8 - direita perto - rápido direita
      }
      else if (mediaCentro < 200 && mediaCentro > 51)
      {
        vel_motor_1 = 600;
        vel_motor_2 = 300;
        enemyfront = 0;
        // 9 - direita longe - lento direita
      }
      else if (mediaCentro >= 200)
      {
        vel_motor_1 = 250;
        vel_motor_2 = 100;
        enemyfront = 0;
      }
      else
      {
        enemyfront = 0;
      }
    }
    if (mediaCentro != -9999)
    {
      lastMediaCentro = mediaCentro;
    } // marca a "memoria"
    if (enemyfront)
    {
      totalFrente();
    }
  }
}

void re()
{
  current_time = millis();
  if (current_time - start_time < tempoRe && flagRe)
  {
    // se o tempo de ré não tiver passado ele ajeita o robo e volta reto
    if (border_dir && border_esq)
    {
      tempoRe = 300;
      vel_motor_1 = -700;
      vel_motor_2 = -700;
    }
    else if (border_dir)
    {
      tempoRe = 300;
      vel_motor_1 = -200;
      vel_motor_2 = -800;
    }
    else if (border_esq)
    {
      tempoRe = 300;
      vel_motor_1 = -800;
      vel_motor_2 = -200;
    }
    else
    {
      vel_motor_1 = -700;
      vel_motor_2 = -700;
    }
  }
  else
  { // zera as variaveis
    tempoRe = 0;
    flagRe = 0;
  }
}

void check_border()
{
  if (line_detected != last_line_detected) // testa pra caso esteja realmente vendo a linha
  {
    start_time = millis();
  }
  last_line_detected = line_detected;
  if ((border_dir || border_esq) && enemyfront == 0)
  { // se viu qualquer borda inicializa as variaveis pra entrar na preferencia da ré e ter
    // "tempo" de arrumar
    tempoRe = 200;
    line_detected = 1;
    flagRe = 1;
  }
  else
  {
    line_detected = 0;
  }
}

int calculateDistance(int distances[])
{
  // joga pesos em cada sensor multiplicando
  int Media[NUM_SENSORS] = {50, 5, -5, -50}, distanciaP = 0, distanciaN = 0;
  for (int i = 0; i <= NUM_SENSORS; i++)
  {
    // alterar para 300 pro segue mão de teste
    // 500 na luta
    if (distances[i] > 400)
    {
      distances[i] = 0;
    }
    Media[i] = distances[i] * Media[i];
  }
  // soma as partes positivas e negativas dividindo pela soma dos pesos
  distanciaP = (Media[0] + Media[1]) / 55;
  distanciaN = (Media[2] + Media[3]) / 55;
  if (distanciaP == 0 && distanciaN == 0)
  {
    return -9999;
  }                                 // se tiver zerado então ta fora do raio do sensor
  return (distanciaP + distanciaN); // retorna a soma do lado positivo e negativo
}

void meiaLua()
{ // estratégia que gira em meia lua por certo tempo
  current_time = millis();
  if (current_time - start_timeStrategy <= strategyTime)
  {
    vel_motor_1 = 1000;
    vel_motor_2 = 600;
  }
  else
  {
    strategyDone = 1;
  }
}

void frenteUmPouco()
{ // vai pra frente por um tempo estimulado
  current_time = millis();
  if (current_time - start_timeStrategy <= strategyTime)
  {
    vel_motor_1 = 400;
    vel_motor_2 = 400;
  }
  else
  {
    strategyDone = 1;
  }
}

void totalFrente()
{
  // REVIEW: excluido código comentado não utilizado mais
  if (!startFrente_flag)
    start_timeFrente = millis();
  startFrente_flag = 1;
  if (millis() - start_timeFrente >= frenteTime)
  {
    vel_motor_1 = 800;
    vel_motor_2 = 800;
  }
}

void strategy_selector()
{
  if (!strategyStart)
  {
    start_timeStrategy = millis();
    strategyStart = 1;
  }
  if (!strategyDone)
  {
    switch (strategy)
    {
    case S0:
      strategyTime = 100;
      frenteUmPouco();
      break;
    case S1:
      /*
        strategyTime = 3000;
        meiaLua();
        break;
        case S2:
        strategyTime = 60000;
        meiaLua();*/
      vel_motor_1 = 1000;
      vel_motor_2 = 1000;
      break;
    }
  }
} //
// parte da divisão dos cores pra leitura do vl
void updateCalculatedDistance()
{
  int distances[NUM_SENSORS];
  int mediaDistancias = 0;
  // Tenta ler da fila sem bloquear
  if (xQueueReceive(distanceQueue, &distances, 0))
  {
    // Calcula o valor com base nas leituras dos sensores
    //  calculatedDistance = calculateDistance(distances);
    //}
    for (int i = 0; i < 5; i++) // REVIEW: Isso está levando muito tempo? Temos como diminuir? Precisa?
    {
      mediaDistancias += calculateDistance(distances);
    }
    calculatedDistance = mediaDistancias / 5;
    mediaDistancias = 0;
  }
}

void printCalculatedDistance()
{
  Serial.print("Calculated Distance: ");
  Serial.println(calculatedDistance);
}
void readSensorsTask(void *pvParameters)
{
  int distances[NUM_SENSORS];
  while (1)
  {
    sensores.distanceRead();
    for (int i = 0; i < NUM_SENSORS; i++)
    {
      distances[i] = sensores.dist[i];
    }
    // Envia as distâncias para a fila sem bloquear
    xQueueSendFromISR(distanceQueue, &distances, NULL);
  }
}