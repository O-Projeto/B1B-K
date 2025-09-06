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
refletancia  qr_dir(qrDir, 3000);
refletancia  qr_esq(qrEsq, 3000);
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

bool evitarBordaAtivado = true;
bool servoNaPosicaoDeAtaque = false; // Controla a posição do servo

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

    if (comandoReal == ONE) LED.blink(1000, VERDE);
    if (comandoReal == SEVEN) {
        evitarBordaAtivado = !evitarBordaAtivado;
        LED.latch(300, evitarBordaAtivado ? VERDE : VERMELHO);
    }

    if (comandoReal == ONE || comandoReal == TWO || comandoReal == TREE) {
        ultimoComandoDeAcao = comandoReal;
    }
    
    if (comandoReal == -1) {
        comandoEfetivo = ultimoComandoDeAcao;
    }

    switch (comandoEfetivo) {
        case ONE:
            mudarEstado(AGUARDANDO_INICIO);
            break;
        case TWO:
            if (estadoAtual == AGUARDANDO_INICIO) {
                mudarEstado(BUSCANDO);
            }
            break;
        case TREE:
            mudarEstado(FIM_DE_PARTIDA);
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

            if (bordaDetectada && evitarBordaAtivado) { mudarEstado(EVITANDO_BORDA); return; }
            if (!inimigoAVista) { mudarEstado(BUSCANDO); return; }

            for (int i = 0; i < NUM_SENSORS; i++) lastRead[i] = sensor.sensorRead[i];

            if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_LATERAL_ESQ] && sensor.sensorRead[SENSOR_FRENTE_ESQ]) {
                vel_motor_dir = 400; vel_motor_esq = 800;
            } else if (sensor.sensorRead[SENSOR_FRENTE_DIR] && sensor.sensorRead[SENSOR_LATERAL_DIR]) {
                vel_motor_dir = 800; vel_motor_esq = 400;
            } else if (sensor.sensorRead[SENSOR_LATERAL_ESQ]) {
                vel_motor_dir = 0; vel_motor_esq = 600;
            } else if (sensor.sensorRead[SENSOR_LATERAL_DIR]) {
                vel_motor_dir = 600; vel_motor_esq = 0;
            } else if (sensor.sensorRead[SENSOR_FRENTE_CTR]) {
                vel_motor_dir = 800; vel_motor_esq = 800;
            } else if (sensor.sensorRead[SENSOR_FRENTE_DIR]){
                vel_motor_dir = 600; vel_motor_esq = 800;
            } else if (sensor.sensorRead[SENSOR_FRENTE_ESQ]){
                vel_motor_dir = 800; vel_motor_esq = 600;
            }
            break;

        case BUSCANDO: {
            // Garante que o servo está para BAIXO
            if (!servoNaPosicaoDeAtaque) {
                myServo.write(90);
                servoNaPosicaoDeAtaque = true;
            }

            if (bordaDetectada && evitarBordaAtivado) { mudarEstado(EVITANDO_BORDA); return; }
            if (inimigoAVista)  { mudarEstado(ATACANDO); return; }

            unsigned long tempoNoEstadoAtual = millis() - tempoInicioEstado;
            bool temMemoria = false;
            for(int i = 0; i < NUM_SENSORS; i++) if (lastRead[i]) { temMemoria = true; break; }

            if (tempoNoEstadoAtual <= 700) { 
                if (temMemoria) {
                    if (lastRead[SENSOR_LATERAL_ESQ] || lastRead[SENSOR_FRENTE_ESQ]) {
                        vel_motor_dir = -200; vel_motor_esq = 400;
                    } else {
                        vel_motor_dir = 400; vel_motor_esq = -200;
                    }
                } else {
                    vel_motor_dir = 400; vel_motor_esq = 400;
                }
            } else if (tempoNoEstadoAtual <= 1200) {
                vel_motor_dir = 400; vel_motor_esq = 400;
            } else {
                tempoInicioEstado = millis(); 
            }
            break;
        }

        case EVITANDO_BORDA:{
            unsigned long tempo_decorrido = millis() - tempoInicioEstado;
            unsigned int duracao_reacao = 400;

            if (bordaDirDetectada && bordaEsqDetectada) {
                duracao_reacao = 200; vel_motor_dir = -500; vel_motor_esq = -500;
            } else if (bordaDirDetectada) {
                duracao_reacao = 300; vel_motor_dir = 0; vel_motor_esq = -500;
            } else if (bordaEsqDetectada) {
                duracao_reacao = 300; vel_motor_dir = -500; vel_motor_esq = 0;
            } else {
                duracao_reacao = 150; vel_motor_dir = -400; vel_motor_esq = -400;
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
    }
}

