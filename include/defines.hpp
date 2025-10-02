//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>                          // DOIS | Funcoes basicas do framework Arduino
#include <BluetoothSerial.h>                  // DOIS | Comunicacao serial via Bluetooth para debug
#include <FastLED.h>                          // DOIS | Controle de fitas de LED enderecaveis
#include <IRremote.hpp>                       // DOIS | Decodifica sinais de controle remoto IR
#include <math.h>                             // DOIS | Funcoes matematicas padrao (sqrt, pow)

#include "soc/gpio_struct.h"                   // DOIS | Acesso direto e rapido aos pinos GPIO
#include "driver/ledc.h"                       // DOIS | Controle do periferico de PWM por hardware

#pragma endregion

//=====================================//Objetos e Handles//=====================================//

#pragma region OBJETOS E HANDLES

BluetoothSerial                SerialBT;      //  DOIS | Objeto para controlar o BT
CRGB                           leds[5];       //  DOIS | Objeto para os 5 LEDs enderecaveis

hw_timer_t *sensorTimer        = NULL;        //  AUTO | Handle do timer dos JSumos

TaskHandle_t openServoHandle   = NULL;        //  DOIS | Handle da task de abrir servo
TaskHandle_t closeServoHandle  = NULL;        //  DOIS | Handle da task de fechar servo

TaskHandle_t combatLogicHandle = NULL;        //  AUTO | Handle da task do sensor IR
TaskHandle_t stratLutaHandle   = NULL;        //  AUTO | Handle da task da estrategia inicial

TaskHandle_t stopRobotHandle   = NULL;        //  AUTO | Handle da task de desligar robo
TaskHandle_t IRCommandHandle   = NULL;        //  AUTO | Handle da task de interpretar IR
TaskHandle_t swSensorHandle    = NULL;        //  AUTO | Handle da task pra desligar sensor

QueueHandle_t btQueue;                        //  AUTO | Handle da fila do BT

volatile uint16_t ultimoComandoIR = 0xFFFF;   //  DOIS | Armazena o comando IR recebido

// Mux para a leitura de sensores e escrita dos motores
portMUX_TYPE sensorMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE motorMux = portMUX_INITIALIZER_UNLOCKED;

//=========================================//Bluetooth//=========================================//

// Verifica se o Bluetooth esta devidamente habilitado no menuconfig
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif                                        // Finaliza o condicional do erro

#pragma endregion

//========================================//Instrucoes//=========================================//

#pragma region INSTRUCOES

/*
!INFO | Instrucoes e GPIOs do ESP32
-----------------------------------
Definicao da placa
   -> No ArduinoIDE selecionar: ESP32 Dev Module

Definicao de pinos e configuracoes
   O ESP32-WROOM-32 possui 39 GPIOs que podem ser configuradas como entrada, saida, ADC, PWM etc
         
      -> GPIOs que NAO devem ser usadas em projetos:
         - GPIOs 6, 7, 8, 9, 10, 11: conectadas na memoria SPI Flash do ESP32
         - GPIOs 20, 24, 28, 29, 30, 31, 37, 38: usadas internamente, nao disponiveis pra uso

      -> GPIOs com uso limitado ou comportamento especial no boot ou flash:
         - GPIO 0: deve estar em LOW para entrar no modo de flash
         - GPIO 1: conectada ao TX Pin do ESP, usada para comunicacao com o modulo
         - GPIO 2: deve estar flutuando ou em LOW durante o boot. Usar para LED_BUILTIN 
         - GPIO 3: conectada ao RX Pin do ESP, usada para comunicacao com o modulo
         - GPIO 12: causa falha no boot se estiver em nivel logico HIGH (strapping pin)
         - GPIO 15: deve estar em HIGH durante o boot
         - GPIOs 34, 35, 36, 39: INPUT ONLY e nao tem resistores pull-up/down internos

      -> GPIOs recomendadas para uso geral (sem restricoes conhecidas):
         - GPIOs 4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33.

      -> Mais informacoes em: https://tinyurl.com/3p99rpkk
*/

#pragma endregion

//===============================================================================================//
//=====================================//PINOS E VARIAVEIS//=====================================//
//===============================================================================================//

//==========================================//Motores//==========================================//

#pragma region PINOS MOTORES

/*
!INFO | Ultimos pinos conhecidos dos robos
-----------------------------------
              LP     LN     RP     RN
SMOKER        17     16     19     18
ARRUELA
BRIGA         19     18     16     17
FUEGO 
FUEGUITO
RESSACA
SHENLONG 
TSUNAMI 
VAMPETA  
*/

#define LEFT_POS_PIN           17             //  DOIS | Ponte H M1 A_IN_2 ESP32 IO18
#define LEFT_NEG_PIN           16             //  DOIS | Ponte H M1 A_IN_1 ESP32 IO19
#define RIGHT_POS_PIN          19             //  DOIS | Ponte H M2 B_IN_2 ESP32 IO16
#define RIGHT_NEG_PIN          18             //  DOIS | Ponte H M2 B_IN_1 ESP32 IO17

#define LEFT_POS_CHANNEL       LEDC_CHANNEL_6 //  DOIS | Canal PWM positivo esquerdo
#define LEFT_NEG_CHANNEL       LEDC_CHANNEL_7 //  DOIS | Canal PWM negativo esquerdo
#define RIGHT_POS_CHANNEL      LEDC_CHANNEL_4 //  DOIS | Canal PWM positivo direito
#define RIGHT_NEG_CHANNEL      LEDC_CHANNEL_5 //  DOIS | Canal PWM negativo direito

#define PWM_FREQ               500            //  DOIS | Frequencia para os motores (Hz)
#define PWM_RESOLUTION         8              //  DOIS | Resolucao de 8 bits
#define PWM_ZERO_DELAY         30             //  DOIS | Tempo para Ponte H limpar PWM (µs)

#pragma endregion

//========================================//Servomotor//========================================//

#pragma region PINOS SERVO

#define SERVOMOTOR_PIN         26             //  DOIS | Servomotor da haste ESP32 IO32

#define SERVO_LEDC_CHANNEL     LEDC_CHANNEL_0 //  DOIS | Canal do PWM do servo
#define SERVO_TIMER            LEDC_TIMER_0   //  DOIS | Timer do PWM do servo

#define SERVO_FREQ_HZ          50             //  DOIS | Frequencia padrao do MG90S
#define SERVO_RESOLUTION       16             //  DOIS | Resolucao de 16 bits para o canal
#define SERVO_MIN_PULSE_US     544            //  DOIS | Largura de pulso para 0 graus (µs)
#define SERVO_MAX_PULSE_US     2400           //  DOIS | Largura de pulso para 180 graus (µs)

#pragma endregion

//========================================//Comunicacao//========================================//

#pragma region COMUNICACAO

#define LED_PIN                2              //  DOIS | LED padrão ESP32 IO2
#define IR_RECIEVE_PIN         5              //  DOIS | Sensor IR ESP32 IO13

#define LEDS_ENDERECAVEIS_PIN  33             //  DOIS | LEDs enderecaveis ESP32 IO33
#define NUM_LEDS               5              //  DOIS | Numero de LEDs enderecaveis

#define MAX_STEPS              20             //  AUTO | Numero maximo de passos da estrategia z
#define BT_BUFFER_SIZE         64             //  AUTO | Tamanho buffer de comandos personalizados
#define BT_QUEUE_LENGTH        128            //  AUTO | Tamanho dda fila de BT

#pragma endregion

//=======================================//Sensoreamento//=======================================//

#pragma region SENSOREAMENTO

#define TRANSISTOR_SENSOR_PIN  25             //  AUTO | Transistor que desliga o IR

#define JSUMO_D_PIN            36             //  AUTO | JSumo direito ESP32 IO36
#define JSUMO_F_PIN            34             //  AUTO | JSumo frontal ESP32 IO34
#define JSUMO_E_PIN            39             //  AUTO | JSumo esquerdo ESP32 IO39

#define SENSOR_IR_D            4              //  AUTO | Sensor IR direito ESP32 IO4
#define SENSOR_IR_F            23             //  AUTO | Sensor IR frontal ESP32 IO23
#define SENSOR_IR_E            13             //  AUTO | Sensor IR esquerdo ESP32 IO5

#pragma endregion

//===================================//Variaveis utilizadas//====================================//

#pragma region VARIAVEIS

volatile bool hasteAbaixada    = false;       //  DOIS | Define se a haste do robo vai ser abaixada
volatile bool valueJsumoF      = false;       //  AUTO | Definicao do Jsumo Frontal
volatile bool valueJsumoE      = false;       //  AUTO | Definicao do Jsumo Esquerdo
volatile bool valueJsumoD      = false;       //  AUTO | Definicao do Jsumo Direito

bool inicializado              = false;       //  DOIS | Flag para validar a inicializacao 
bool hasteForcada              = true;        //  DOIS | Define se a haste deve ficar ligada aberta
bool isConfiguring             = false;       //  AUTO | Flag para controlar o modo de configuracao
bool running                   = false;       //  AUTO | Indica se o robo esta lutando
bool ready                     = false;       //  AUTO | Usada para testar se o robo recebe IR
bool seeing                    = false;       //  AUTO | Usada para indicar se o robo ve o outro
bool modoFurtivo               = false;       //  AUTO | Usada para indicar se deve desligar Jsumo

unsigned long tempoCombat = 0;

volatile int novaVE            = 0;           //  DOIS | Velocidade pra atualizar o motor esquerdo
volatile int novaVD            = 0;           //  DOIS | Velocidade pra atualizar o motor direito

int customStrategyCount        = 0;           //  AUTO | Contagem de passos da estrategia z
int bufferIndex                = 0;           //  AUTO | Indice para controlar a posicao no buffer

char BT                        = 'x';         //  AUTO | Caractere recebido pelo BT
char strategy                  = 'x';         //  AUTO | Estrategia da luta (qualquer caracter)
char btBuffer[BT_BUFFER_SIZE];                //  AUTO | Buffer para os comandos personalizados

#pragma endregion

//======================================//Enums e Structs//======================================//

#pragma region ENUMS E STRUCTS

enum SelecionouModo {                         //  DOIS | Define se o modo de luta foi selecionado
   naoSelecionado,
   autonomo,
   radioControlado
};

enum ModoLuta {                               //  AUTO | Define qual o modo de luta iterativo
   defesa,
   ataque,
   busca,
   giro,
   quebrado,
   woodpecker,
   slowsearch
};

enum Direction {                              //  AUTO | Direcao de movimentacao do robo
   esquerda,
   direita,
   reto
};

enum DirecaoAdversario {                      //  AUTO | Direcao de movimentacao do adversario
   vistoEsquerda,
   vistoDireita,
   nuncaVisto
};

struct StrategyStep {                         //  AUTO | Struct para os comandos personalizados
   int speedLeft;
   int speedRight;
   int delayMs;
};

SelecionouModo selecionouModo  = naoSelecionado;
ModoLuta modoLuta              = ataque;
Direction direction            = esquerda;
DirecaoAdversario ultimoLado   = nuncaVisto;
StrategyStep customStrategy    [MAX_STEPS];

#pragma endregion

//===============================================================================================//
//=====================================//FINALIZA O ARQUIVO//====================================//
//===============================================================================================//

#endif