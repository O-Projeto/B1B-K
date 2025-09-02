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
JS40F_JSumo sensor;
controle_juiz controle_sony(34);
refletancia qr_dir(qrDir, 3000);
refletancia qr_esq(qrEsq, 3000);
led_rgb LED;
Motor450 motordir = Motor450(AIN1, AIN2, offsetA, offsetB, 10);
Motor450 motoresq = Motor450(BIN1, BIN2, offsetC, offsetD, 10);
Servo myServo;

// =================================================================
// 2. ESTADOS DO ROBÔ
// =================================================================
enum EstadoRobo {
    AGUARDANDO_INICIO,
    BUSCANDO,
    ATACANDO,
    EVITANDO_BORDA,
    FIM_DE_PARTIDA // NOVO ESTADO: Trava o robô permanentemente
};

// =================================================================
// 3. VARIÁVEIS DE ESTADO E CONTROLE
// =================================================================
EstadoRobo estadoAtual = AGUARDANDO_INICIO;
unsigned long tempoInicioEstado = 0;

int vel_motor_dir = 0;
int vel_motor_esq = 0;

int lastRead[NUM_SENSORS] = {0};
int ultimoComandoIR = 0;

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
    myServo.write(90);

    LED.blink(AZUL, 1000);
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
    delay(10); // Um pequeno delay pode ajudar na estabilidade
}

// =================================================================
// 6. LÓGICA DA MÁQUINA DE ESTADOS
// =================================================================

// Função centralizada para mudar de estado.
void mudarEstado(EstadoRobo novoEstado) {
    if (estadoAtual != novoEstado) {
        estadoAtual = novoEstado;
        tempoInicioEstado = millis(); // Reseta o timer do estado

        // Adiciona feedback visual para cada estado
        switch (novoEstado) {
            case ATACANDO:        LED.set(VERMELHO); break;
            case BUSCANDO:        LED.set(VERDE); break;
            case EVITANDO_BORDA:  LED.set(AMARELO); break;
            case FIM_DE_PARTIDA:  LED.set(AZUL); break; // Azul sólido para indicar fim
            case AGUARDANDO_INICIO: LED.latch(AZUL, 500); break;
        }
    }
}

void executarMaquinaDeEstados() {
    // --- Leituras de sensores feitas uma vez por ciclo ---
    sensor.distanceRead();
    bool bordaDirDetectada = qr_dir.detect_border();
    bool bordaEsqDetectada = qr_esq.detect_border();
    bool bordaDetectada = bordaDirDetectada || bordaEsqDetectada;

    bool inimigoAVista = false;
    for(int i=0; i < NUM_SENSORS; i++) {
        if (sensor.sensorRead[i]) {
            inimigoAVista = true;
            break;
        }
    }

    // Se virmos o inimigo, atualizamos a memória
    if (inimigoAVista) {
        for (int i = 0; i < NUM_SENSORS; i++) {
            lastRead[i] = sensor.sensorRead[i];
        }
    }

    // --- Lógica de Transição e Ação para cada estado ---
    switch (estadoAtual) {
        case AGUARDANDO_INICIO:
            vel_motor_dir = 0;
            vel_motor_esq = 0;
            break;

        case ATACANDO:
            if (bordaDetectada) { mudarEstado(EVITANDO_BORDA); return; }
            if (!inimigoAVista) { mudarEstado(BUSCANDO); return; }

            // AÇÃO: Lógica de ataque
            if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_FRENTE_ESQ] && sensor.sensorRead[SENSOR_FRENTE_CTR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_FRENTE_CTR] && sensor.sensorRead[SENSOR_FRENTE_DIR]) {
                vel_motor_dir = 1000; vel_motor_esq = 1000;
            } else if (sensor.sensorRead[SENSOR_LATERAL_ESQ] && sensor.sensorRead[SENSOR_FRENTE_ESQ]) {
                vel_motor_dir = 300; vel_motor_esq = 600;
            } else if (sensor.sensorRead[SENSOR_FRENTE_DIR] && sensor.sensorRead[SENSOR_LATERAL_DIR]) {
                vel_motor_dir = 600; vel_motor_esq = 300;
            } else if (sensor.sensorRead[SENSOR_LATERAL_ESQ]) {
                vel_motor_dir = 0; vel_motor_esq = 500;
            } else if (sensor.sensorRead[SENSOR_LATERAL_DIR]) {
                vel_motor_dir = 500; vel_motor_esq = 0;
            } else if (sensor.sensorRead[SENSOR_FRENTE_CTR]) {
                vel_motor_dir = 800; vel_motor_esq = 800;
            }
            break;

        case BUSCANDO:
            if (bordaDetectada) { mudarEstado(EVITANDO_BORDA); return; }
            if (inimigoAVista) { mudarEstado(ATACANDO); return; }

            // AÇÃO: Lógica de busca
            if (millis() - tempoInicioEstado > 500) {
                vel_motor_dir = 400;
                vel_motor_esq = 400;
                for(int i = 0; i < NUM_SENSORS; i++) { lastRead[i] = 0; }
            } else {
                if (lastRead[SENSOR_LATERAL_ESQ] || lastRead[SENSOR_FRENTE_ESQ]) {
                    vel_motor_dir = 400; vel_motor_esq = -200; // Vira para esquerda
                } else if (lastRead[SENSOR_FRENTE_DIR] || lastRead[SENSOR_LATERAL_DIR]) {
                    vel_motor_dir = -200; vel_motor_esq = 400; // Vira para direita
                } else {
                    vel_motor_dir = 400; vel_motor_esq = -400; // Gira
                }
            }
            break;

        case EVITANDO_BORDA:
            unsigned long tempo_decorrido = millis() - tempoInicioEstado;
            unsigned int duracao_reacao = 200; // Duração padrão

            if (bordaDirDetectada && bordaEsqDetectada) {
                duracao_reacao = 200; vel_motor_dir = -500; vel_motor_esq = -500;
            } else if (bordaDirDetectada) {
                duracao_reacao = 300; vel_motor_dir = -700; vel_motor_esq = -250;
            } else if (bordaEsqDetectada) {
                duracao_reacao = 300; vel_motor_dir = -250; vel_motor_esq = -500;
            } else {
                duracao_reacao = 150; vel_motor_dir = -400; vel_motor_esq = -400;
            }

            if (tempo_decorrido >= duracao_reacao) {
                mudarEstado(BUSCANDO);
            }
            break;
        
        // NOVO CASE: Garante que o robô permaneça parado.
        case FIM_DE_PARTIDA:
            vel_motor_dir = 0;
            vel_motor_esq = 0;
            myServo.write(90); // Opcional: retorna o servo à posição inicial
            break;
    }
}

void processarComandoIR() {
    // ALTERAÇÃO: Se a partida terminou, ignora qualquer novo comando.
    if (estadoAtual == FIM_DE_PARTIDA) {
        return;
    }

    int comando = controle_sony.read();

    if (comando != -1) {
        switch (comando) {
            case ONE:
                // Só funciona se estiver no estado inicial.
                if (estadoAtual == AGUARDANDO_INICIO) {
                    mudarEstado(AGUARDANDO_INICIO);
                    myServo.write(90);
                }
                break;

            case TWO:
                if (estadoAtual == AGUARDANDO_INICIO) {
                    myServo.write(180); // Posição de ataque
                    mudarEstado(BUSCANDO);
                }
                break;

            case TREE:
                // ALTERAÇÃO: Muda para o estado final e sem retorno.
                mudarEstado(FIM_DE_PARTIDA);
                break;
        }
    }
}
