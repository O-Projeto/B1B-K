# B1B-K

#todo - 
Deixar as libs em devices!!!

NA fight deixar o enemy tracker [:s]
e diferentes códigos de luta [:s]
Funciona controle [:s] 
Leitura vl no segundo core
Trashhold das duas b1b-ks  
PID!!!!
Funciona ré  
para integrar dps 
snake_case, primeira letra maiuscula- nome da pasta de origem

Devices-vls
Devices-rgb
Devices-motor
Devices-refletancia

Fight-enemy_tracker
Main-main

documentar no começo do código que é importante verificar qual das b1b-ks por conta dos sensores

Validar eletrônica dos dois, deixar motores iguais


README para saber como ler a prog da B1B-K.
## Bibliotecas

Controle dos motores: H_bridge_TB6612.hpp 

Leitura dos QR: refletancia.h

Leitura do controle: controle_juiz.h

Leitura dos VL: VL53_sensors.h

Controle do LED: led_rgb.h

Filtro de mediana: MedianFilter.h

# Main - Como entender

## Início do Loop
No início do loop são feitas as leituras dos:
Controle do Juiz
QRs
## Maquina de estados - Comandos do Controle:
Manual de Comandos do Controle:

1 - Verifica Comuninacação - Pisca Verde

2 - Começa a batalha

3 - Para o robô

4, 5 e 6 - Seleção de estratégias

0 - Avisa a B1B-K que tem bandeira 
```c
switch (read_ir) {
    case ONE: // estado para ver se o controle está comunicando
      last_ir = ONE;
      LED.latch(200, VERDE);
      start time = millis();
      break;
    
    case TWO: // Start
      current_time = millis();
      sensores.distanceRead(); // Le os VL
      sensores.printDistancesBT(); 
      strategy_selector(); // Seleciona a estratégia
      attack(); // Ataca caso capte os VL
      check_border(); // Verifica as bordas
      leftMotor.drive(left_vel);
      delay(10);
      rightMotor.drive(right_vel);
      delay(10);
      LED.set(MAGENTA); 
      last_ir = TWO;
      break;
    case TREE: // Parar
      // Desliga Mototes
      leftMotor.drive(0);
      delay(10);
      rightMotor.drive(0);
      delay(10);
      LED.set(VERMELHO);
      last_ir = TREE;
      start time = millis();
      break;
    case BANDEIRA: // Avisa que tem bandeira
      bandeira flag = true;
      LED.set(AMARELO);
      last_ir = BANDEIRA;
      start time = millis();
      break;
    case FOUR:
      strategy = S0;
      start time = millis();
      // tempoPrevio = 0;
      last_ir = FOUR;
      LED.latch(200, AZUL);
      break;
    case FIVE:
      strategy = S1;
      start time = millis();
      LED.latch(200,VERMELHO);
      last_ir = FIVE;
      break;
    case SIX:
      strategy = S2;
      start time = millis();
      LED.latch(200, LARANJA);
      last_ir = SIX;
      break;
    default:
      start time = millis();
      LED.set(0);
      break;
  }
  read_ir = controle.read();
}
```


## Verificação de Borda

A verificação de borda é feita na função check_border() chamada no loop após o start. Ela se movimenta diferente caso identifique os dois ou apenas um dos sensores QR. Se detectar em ambos ela irá andar de ré, caso somente um, fará mais como uma curva no próprio eixo desviando da borda. 

A check_border tem que ser a última função a ser chamada, para que a B1B-K não se suicide.

## Identificação do oponente

A identificação do oponente é feita na função attack, a B1B-K está programada para atacar com mais velocidade a 5 cm do robô, com menor a 10 cm do robô logo a sua frente, além de se direcionar dependendo de quais sensores do VL ela capta o robô inimigo. 

## Seleção das estratégias

A função strategy_selector seleciona a movimentação a partir das estratégias selecionadas pelo controle, S0, S1 e S2.