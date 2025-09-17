#include <Arduino.h>
#include "config.hpp"
#include "controle_juiz.hpp"
#include "refletancia.h"
#include "led_rgb.h"
#include "JS40F_JSumo.h"
#include "Motor450.h"
#include <ESP32Servo.h>

// =================================================================
// 1. OBJETOS DE HARDWARE
// =================================================================
JS40F_JSumo  sensor;
controle_juiz controle_sony(34);
refletancia  qr_dir(qrDir, 300);
refletancia  qr_esq(qrEsq, 300);
led_rgb      LED;
Motor450     motordir = Motor450(AIN1, AIN2, offsetA, offsetB, 10);
Motor450     motoresq = Motor450(BIN1, BIN2, offsetC, offsetD, 10);
Servo        myServo;

// =================================================================
// 2. ESTADOS DO ROBÔ
// =================================================================
enum EstadoRobo {
    AGUARDANDO_INICIO,
    BUSCANDO,
    ATACANDO,
    EVITANDO_BORDA,
    FRENTE_UM_POUCO,
    DELAY_1SEC,
    MEIA_LUA,
    ZIGUE_ZAGUE,
    DE_COSTAS,
    FIM_DE_PARTIDA
};

// =================================================================
// 3. VARIÁVEIS DE ESTADO E CONTROLE
// =================================================================
EstadoRobo estadoAtual = AGUARDANDO_INICIO;
unsigned long tempoInicioEstado = 0;

int vel_motor_dir = 0;
int vel_motor_esq = 0;

int lastRead[NUM_SENSORS] = {0};
int ultimoComandoDeAcao = 0;

bool bordaAtivado = true;
bool servoNaPosicaoDeAtaque = false; // Controla a posição do servo

bool delaySensor = true;
bool frente = false;
bool delay_1seg = false;
bool meiaLua = false;
bool zigueZague = false;
bool deCostas   = false;
// =================================================================
// 4. PROTÓTIPOS E SETUP
// =================================================================
void mudarEstado(EstadoRobo novoEstado);
void executarMaquinaDeEstados();
void processarComandoIR();

void drive(int mot1, int mot2) {
    motordir.drive(mot1);
    motoresq.drive(mot2);
}

void setup() {
    Serial.begin(115200);
    sensor.sensorsInit();
    controle_sony.init();
    LED.init();
    
    myServo.attach(SERVO);
    myServo.write(0); // Posição de repouso (para CIMA)
    servoNaPosicaoDeAtaque = false;

    LED.set(BRANCO);
    delay(1000);
    mudarEstado(AGUARDANDO_INICIO);
}

// =================================================================
// 5. LOOP PRINCIPAL
// =================================================================
void loop() {
    processarComandoIR();
    executarMaquinaDeEstados();
    drive(vel_motor_dir, vel_motor_esq);
}

// =================================================================
// 6. FUNÇÕES DE CONTROLE
// =================================================================
void mudarEstado(EstadoRobo novoEstado) {
    if (estadoAtual != novoEstado) {
        estadoAtual = novoEstado;
        tempoInicioEstado = millis();
    }
}

void processarComandoIR() {
    if (estadoAtual == FIM_DE_PARTIDA) return;

    int comandoReal = controle_sony.read();

    int comandoEfetivo = comandoReal;
    if (comandoReal == ONE || comandoReal == TWO || comandoReal == TREE) {
        ultimoComandoDeAcao = comandoReal;
    }
    
    if (comandoReal == -1) {
        comandoEfetivo = ultimoComandoDeAcao;
    }

    switch (comandoEfetivo) {
        case ONE:
             if (comandoReal == ONE) {LED.blink(MAGENTA);}//{LED.blinkAlternado(MAGENTA, AZUL);}
             break;
        case TWO:
            if (delaySensor) {
                delay(50); delaySensor = false;}
            if (estadoAtual == AGUARDANDO_INICIO) {
                mudarEstado(BUSCANDO);
            }
            break;
        case TREE:
            mudarEstado(FIM_DE_PARTIDA);
            break;
        case HOME:
            if (estadoAtual == AGUARDANDO_INICIO){
                frente = true;
                LED.set(LARANJA);
            }
             break;
        case RETURN:
            bordaAtivado = false;
            LED.set(VERDE);
            break;
        case OPTIONS: 
            delay_1seg = true;
            LED.set( CIANO);
            break;
        case DISPLAY:
            meiaLua = true;
            LED.set(VERMELHO);
            break;
        case GUIDE:
            zigueZague = true;
            LED.set(BRANCO);
            break;
        case SYNC_MENU:
            deCostas = true;
            LED.set(AMARELO);
            break;
    }
}

void executarMaquinaDeEstados() {
    bool bordaDirDetectada = false, bordaEsqDetectada = false, bordaDetectada = false, inimigoAVista = false;

    if (estadoAtual == ATACANDO || estadoAtual == BUSCANDO || estadoAtual == EVITANDO_BORDA) {
        sensor.distanceRead();
        bordaDirDetectada = qr_dir.detect_border();
        bordaEsqDetectada = qr_esq.detect_border();
        bordaDetectada = bordaDirDetectada || bordaEsqDetectada;
        for(int i=0; i < NUM_SENSORS; i++) {
            if (sensor.sensorRead[i]) {
                inimigoAVista = true;
                break;
            }
        }
    }


    switch (estadoAtual) {
        case AGUARDANDO_INICIO:
            vel_motor_dir = 0;
            vel_motor_esq = 0;
            // Garante que o servo está para CIMA
            if (servoNaPosicaoDeAtaque) {
                myServo.write(0);
                servoNaPosicaoDeAtaque = false;
            }
            break;

        case ATACANDO:
            // Garante que o servo está para BAIXO
            if (!servoNaPosicaoDeAtaque) {
                myServo.write(90);
                servoNaPosicaoDeAtaque = true;
            }

            if (bordaDetectada && bordaAtivado) { mudarEstado(EVITANDO_BORDA); return; }
            if (!inimigoAVista) { mudarEstado(BUSCANDO); return; }
            for (int i = 0; i < NUM_SENSORS; i++) lastRead[i] = sensor.sensorRead[i];

            if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_LATERAL_ESQ] && sensor.sensorRead[SENSOR_FRENTE_ESQ]) {
                vel_motor_dir = 800; vel_motor_esq = 500;
            } else if (sensor.sensorRead[SENSOR_FRENTE_DIR] && sensor.sensorRead[SENSOR_LATERAL_DIR]) {
                vel_motor_dir = 400; vel_motor_esq = 800;
            } else if (sensor.sensorRead[SENSOR_LATERAL_ESQ]) {
                vel_motor_dir = 600; vel_motor_esq = 0;
            } else if (sensor.sensorRead[SENSOR_LATERAL_DIR]) {
                vel_motor_dir = 0; vel_motor_esq = 700;
            } else if (sensor.sensorRead[SENSOR_FRENTE_CTR]) {
                vel_motor_dir = 800; vel_motor_esq = 800;
            } else if (sensor.sensorRead[SENSOR_FRENTE_DIR]){
                vel_motor_dir = 300; vel_motor_esq = 800;
            } else if (sensor.sensorRead[SENSOR_FRENTE_ESQ]){
                vel_motor_dir = 800; vel_motor_esq = 700;
            }
            break;

        case BUSCANDO: {
            // Garante que o servo está para BAIXO
            if (!servoNaPosicaoDeAtaque) {
                myServo.write(90);
                servoNaPosicaoDeAtaque = true;
            }
            if (frente) {mudarEstado(FRENTE_UM_POUCO);return;}
            if (delay_1seg){ mudarEstado(DELAY_1SEC);}
            if (zigueZague){mudarEstado(ZIGUE_ZAGUE);}
            if (deCostas) {mudarEstado(DE_COSTAS);}
            if(meiaLua){mudarEstado(MEIA_LUA);}
            if (bordaDetectada && bordaAtivado) { mudarEstado(EVITANDO_BORDA); return; }
            if (inimigoAVista)  { mudarEstado(ATACANDO); return; }
           // unsigned long tempoNoEstadoAtual = millis() - tempoInicioEstado;
            bool temMemoria = false;
            for(int i = 0; i < NUM_SENSORS; i++) if (lastRead[i]) { temMemoria = true; break; }
                if (temMemoria) {
                    if (lastRead[SENSOR_LATERAL_ESQ] || lastRead[SENSOR_FRENTE_ESQ]) {
                        vel_motor_dir = 300; vel_motor_esq = 0;
                    } else if (lastRead[SENSOR_LATERAL_DIR] || lastRead[SENSOR_FRENTE_DIR]){
                        vel_motor_dir = 0; vel_motor_esq = 500;
                    } else {vel_motor_dir = 200; vel_motor_esq = 200;}
                } else {
                    vel_motor_dir = 0; vel_motor_esq = 0;
                }
            break;
        }

        case EVITANDO_BORDA:{
            unsigned long tempo_decorrido = millis() - tempoInicioEstado;
            unsigned int duracao_reacao = 700;

            if (bordaDirDetectada && bordaEsqDetectada) {
                duracao_reacao = 200; vel_motor_dir = -500; vel_motor_esq = -700;
            } else if (bordaDirDetectada) {
                duracao_reacao = 400; vel_motor_dir = -800; vel_motor_esq = 0;
            } else if (bordaEsqDetectada) {
                duracao_reacao = 300; vel_motor_dir = 0; vel_motor_esq = -600;
            } else {
                duracao_reacao = 600; vel_motor_dir = -400; vel_motor_esq = -400;
            }

            if (tempo_decorrido >= duracao_reacao) {
                mudarEstado(BUSCANDO);
            }
            break;
        }
        
        case FIM_DE_PARTIDA:{
            vel_motor_dir = 0;
            vel_motor_esq = 0;
            // Garante que o servo está para CIMA
            if (servoNaPosicaoDeAtaque) {
                myServo.write(0);
                servoNaPosicaoDeAtaque = false;
                LED.set(VERMELHO);
            }
            break;
        }

        case FRENTE_UM_POUCO:{
            drive(300, 700);
            delay(100);
            drive(0,0);
            frente = false;
            mudarEstado(BUSCANDO);
            break;
        }
        case DELAY_1SEC:{
            drive(0,0);
            delay(350);
            delay_1seg = false;
            mudarEstado(FRENTE_UM_POUCO);
            break;
        }
        case MEIA_LUA: {
            drive (400,500);
            delay(200);
            meiaLua =false;
            mudarEstado(BUSCANDO);   
            break;
        }
        case ZIGUE_ZAGUE: {
            /*unsigned long tempo_decorrido = millis() - tempoInicioEstado;
            if (tempo_decorrido > 700){
                zigueZague = false;
                mudarEstado(BUSCANDO);
                return;
            }
            if ((tempo_decorrido / 150) % 2 == 0) {
                drive(300, 0); // Curva para a esquerda
            } else {
                drive(0, 700); // Curva para a direita
            }
            drive(200, 600);*/
            drive(0, 700); // Curva para a esquerda
            delay(150);
            drive(300,700);
            delay(75);
            drive(300, 0); // Curva para a direita
            delay(150);
            zigueZague = false;
            mudarEstado(BUSCANDO);
            break;
    }
        case DE_COSTAS: {
            drive(-300, 700);
            delay(300);
            deCostas = false;
            mudarEstado(BUSCANDO);
            break;
        }
    }

}

