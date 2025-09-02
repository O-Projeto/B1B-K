#include <Arduino.h>
#include "config.hpp"
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"
#include "JS40F_JSumo.h"
#include "Motor450.h"
#include <ESP32Servo.h>  


// // inicialização dos objetos // //
// sensores visão
JS40F_JSumo sensor;
// controle remoto (ir)
controle_juiz controle_sony(34);
// qre (arrumar segundo parametro apos medir na competião)
refletancia qr_dir(qrDir, 3000);
refletancia qr_esq(qrEsq, 3000); // rcx era entre 50 e 100
// led 
led_rgb LED;
// motores 
Motor450 motordir = Motor450(AIN1, AIN2, offsetA, offsetB,  10);
Motor450 motoresq = Motor450(BIN1, BIN2, offsetC, offsetD,  10);
// servo da bandeira
Servo myServo;  
uint8_t servoAngle = 90;  // Começa em 90°

    // // Variaveis // //
// leitura do controle e memoria da ultima leitura
int read_ir = -1;
int last_ir = 0;
// leituras das bordas
bool border_dir, border_esq; 
// servo
bool servoMoved = 0;
// motores 
int vel_motor_dir;
int vel_motor_esq;
// borda (ré)
bool check_border = 1;
bool reacao_em_andamento = false;
unsigned long start_time_reacao = 0;
bool viu_borda_agora;
// // Variáveis de estado search
int lastRead[NUM_SENSORS] = {0,0,0,0,0};
bool virando = 0;
unsigned long inicioVirada = 0;
const int TEMPO_virada = 500;
bool inimigo_a_vista = false;

// Funções //
void gerenciarBorda();
void search();
void drive(int mot1, int mot2);
void leituraControle ();
void executar();


void setup() {
   // inicialização dos sensores, controles e led
   Serial.begin(112500);
   sensor.sensorsInit(); // sensores zerados na classe
   controle_sony.init();
   LED.init();
   LED.blink(AZUL, 1000);
   delay(1000);
   //LED.set(0);
   myServo.attach(SERVO);  
   myServo.write(90);  // Inicia em 90°
}

void loop(){
    leituraControle();
    executar();
}

void executar(){
   // testa as bordas e salva 
    border_dir = qr_dir.detect_border();
    border_esq = qr_esq.detect_border();
    
    switch (read_ir)
    {
    case ONE:
        last_ir = ONE;
        LED.latch(VERDE, 1000);
        break;
    case TWO:
      sensor.distanceRead();
       if (!servoMoved){myServo.write(servoAngle); servoMoved = 1;}
       if (check_border){gerenciarBorda();}
      search();
      drive(vel_motor_dir,vel_motor_esq);
      break;
    case TREE:
      drive(0, 0);
      if (servoMoved) {
        myServo.write(90); // Retorna à posição inicial
        servoMoved = 0;
      }
      delay(10);
      break;
    case SEVEN:
      check_border = 0;
    default:
        break;
    }
}
void leituraControle (){
    // ler o controle e filtrar se isso não é um numero aleatorio
    // ou manter  ele ligado ou desligado
    read_ir = controle_sony.read();
    if (last_ir == TWO && (read_ir != TREE)){read_ir = TWO;}
    else if (last_ir == TREE){read_ir = TREE;}
}
void search(){
  if (reacao_em_andamento){ 
  // Se estiver recuando da borda, não faz nada de busca ou ataque.
    virando = false; // Cancela qualquer busca se vir a borda
    return;}

     // --- LÓGICA DE DETECÇÃO ---
    if (sensor.sensorRead[0]||sensor.sensorRead[1]||sensor.sensorRead[2]||
    sensor.sensorRead[3] || sensor.sensorRead[4]){
      inimigo_a_vista = true;
      virando = false;// Se vimos o inimigo, cancelamos o modo de busca/virada
      }
    else {inimigo_a_vista = false;}

    if (inimigo_a_vista) {
    // ===================================
    // MODO ATAQUE: O inimigo está visível
    // ==================================
    // Atualiza a última posição conhecida do inimigo (fazemos isso uma vez só)
    for (int i = 0; i < NUM_SENSORES; i++) {
      lastRead[i] = sensor.sensorRead[i];
    }
     // --- LÓGICA DE DECISÃO DE ATAQUE ---
    // A ordem é importante, das condições mais específicas para as mais gerais.

    // 1. Alvo Perfeito (3 sensores frontais)
    if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
      vel_motor_dir = 1000;
      vel_motor_esq = 1000;
    }
    // 2. Alvo bem alinhado (2 sensores frontais)
    else if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR]) {
      vel_motor_dir = 1000; 
      vel_motor_esq = 1000;
    } 
    else if (sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
      vel_motor_dir = 1000; 
      vel_motor_esq = 1000;
    }
    // 3. Inimigo na lateral-frontal
    else if (sensor.sensorRead[SENSOR_LATERAL_ESQ] && sensor.sensorRead[SENSOR_FRENTE_ESQ]) {
        vel_motor_dir = 300;
        vel_motor_esq = 600;
    }
    else if (sensor.sensorRead[SENSOR_FRENTE_DIR] && sensor.sensorRead[SENSOR_LATERAL_DIR]) {
        vel_motor_dir = 600;
        vel_motor_esq = 300;
    }
    // 4. Inimigo bem na lateral (requer pivô)
    else if (sensor.sensorRead[SENSOR_LATERAL_ESQ]) {
      vel_motor_dir = 0;
      vel_motor_esq = 500;
    } 
    else if (sensor.sensorRead[SENSOR_LATERAL_DIR]) {
      vel_motor_dir = 500;
      vel_motor_esq = 0;
    }
    // 5. Inimigo apenas no centro (ataque reto)
    else if (sensor.sensorRead[SENSOR_FRENTE_CTR]) {
      vel_motor_dir = 800;
      vel_motor_esq = 800;
    }
  } else {
    // ========================================================
    // MODO BUSCA: O inimigo foi perdido
    // ========================================================
    
    // Se não está virando ainda, inicia a virada de busca
    if (!virando) {
      virando = true;
      inicioVirada = millis();
    }

    // Se já está virando, verifica se o tempo acabou
    if (virando && (millis() - inicioVirada >= TEMPO_virada)) {
      virando = false; // Cancela a virada
      // Zera o lastRead para não ficar preso na última direção
      for(int i = 0; i < NUM_SENSORES; i++) { lastRead[i] = 0; }
      vel_motor_dir = 400; // Continua reto após a virada (como no seu código)
      vel_motor_esq = 400;
    } 
    
    // Se está no meio do tempo de virada
    if (virando) {
        if (lastRead[SENSOR_LATERAL_ESQ] || lastRead[SENSOR_FRENTE_ESQ]) { // Visto pela esquerda -> vira para esquerda
          vel_motor_dir = 400;
          vel_motor_esq = 0;
        } else if (lastRead[SENSOR_FRENTE_DIR] || lastRead[SENSOR_LATERAL_DIR]) { // Visto pela direita -> vira para direita
          vel_motor_dir = 0;
          vel_motor_esq = 400;
        } else { // Não sabe onde estava (ou no centro) -> Gira
          vel_motor_dir = 400;
          vel_motor_esq = -400;
        }
    }
  }
}
void gerenciarBorda()
{
  // Lê os sensores de borda (assumindo que border_dir e border_esq são atualizados em outro lugar)
  viu_borda_agora = (border_dir || border_esq);
  
  // 1. VERIFICA SE UMA NOVA REAÇÃO DEVE COMEÇAR
  // Se não estamos no meio de uma reação e acabamos de ver a borda...
  if (!reacao_em_andamento && viu_borda_agora) {
    reacao_em_andamento = true;   // ...inicia a reação
    start_time_reacao = millis(); // ...marca o tempo de início
  }

  // 2. EXECUTA A REAÇÃO SE ELA ESTIVER EM ANDAMENTO
  if (reacao_em_andamento) {
    unsigned long tempo_decorrido = millis() - start_time_reacao;
    unsigned int duracao_reacao; // O "tempoRe" desta manobra

    // Define a duração da reação e a velocidade dos motores
    // baseado nos sensores que estão vendo a linha.
    // Esta lógica é idêntica à sua função re().
    if (border_dir && border_esq) { // Viu com os dois sensores
      duracao_reacao = 200;
      vel_motor_dir = -500;
      vel_motor_esq = -500;
    } else if (border_dir) { // Viu com o sensor direito
      duracao_reacao = 300;
      vel_motor_dir = -700;
      vel_motor_esq = -250;
    } else if (border_esq) { // Viu com o sensor esquerdo
      duracao_reacao = 300;
      vel_motor_dir = -250;
      vel_motor_esq = -500;
    } else {
      // Caso a reação tenha iniciado mas o robô não veja mais a linha
      // (ex: por causa da velocidade). Ação de recuo padrão.
      duracao_reacao = 200; // Usa uma duração padrão
      vel_motor_dir = -400;
      vel_motor_esq = -400;
    }

    // 3. VERIFICA SE A REAÇÃO TERMINOU
    // Se o tempo decorrido ultrapassou a duração definida para a manobra...
    if (tempo_decorrido >= duracao_reacao) {
      reacao_em_andamento = false; // ...termina a reação.
      // Neste ponto, a lógica principal do seu robô (ex: procurar o oponente)
      // deve assumir o controle dos motores no próximo ciclo do loop.
    }
  }
}

void drive(int mot1, int mot2){
    motordir.drive(mot1);
    motoresq.drive(mot2);
}