/*/==============================================================================================//
//=======================================//SMOKER v2.5.3//=======================================//
//===============================================================================================//

███████████████████████████████████████████████████████████████████████████████████████████████████
█                                                                                                 █
█                                           __________                                            █
█                                        .~#########%%;~.                                         █
█                                       /############%%;`\                                        █
█                                      /######/~\/~\%%;,;,\                                       █ 
█                                     |#######\    /;;;;.,.|                                      █
█                                     |#########\/%;;;;;.,.|                                      █
█                            XX       |##/~~\####%;;;/~~\;,|       XX                             █
█                          XX..X      |#|  o  \##%;/  o  |.|      X..XX                           █
█                        XX.....X     |##\____/##%;\____/.,|     X.....XX                         █
█                       X.....XX      \#########/\;;;;;;,, /      XX.....X                        █
█                      X |..XX%,.@      \######/%;\;;;;, /      @#%,XX..| X                       █
█                     X |..X  @#%,.@     |######%%;;;;,.|     @#%,.@  X..| X                      █
█                    X  \.X     @#%,.@   |# # # % ; ; ;,|   @#%,.@     X./  X                     █
█                   X# \.X        @#%,.@                  @#%,.@         X./ #                    █
█                   #  X            @#%,.@              @#%,.@           X   #                    █
█                   ##X               @#%,.@          @#%,.@              X ##                    █
█                   `###X               @#%,.@      @#%,.@               ####'                    █
█                   ' ###                 @#%.,@  @#%,.@                 ###`"                    █
█                    . ";"                  @#%.@#%,.@                  ;"` ' .                   █
█                       '                     @#%,.@                    ,.                        █
█                       ` ,                 @#%,.@  @@                 `                          █
█                                            @@@  @@@               .                             █
█                                                                                                 █
█                   #######                              /                                        █
█                 /       ###                          #/                                         █
█                /         ##                          ##                                         █
█                ##        #                           ##                                         █
█                 ###                                  ##                                         █
█                ## ###      ### /### /###     /###    ##  /##      /##  ###  /###                █
█                 ### ###     ##/ ###/ /##  / / ###  / ## / ###    / ###  ###/ #### /             █
█                   ### ###    ##  ###/ ###/ /   ###/  ##/   /    /   ###  ##   ###/              █
█                     ### /##  ##   ##   ## ##    ##   ##   /    ##    ### ##                     █
█                       #/ /## ##   ##   ## ##    ##   ##  /     ########  ##                     █
█                        #/ ## ##   ##   ## ##    ##   ## ##     #######   ##                     █
█                         # /  ##   ##   ## ##    ##   ######    ##        ##                     █
█               /##        /   ##   ##   ## ##    ##   ##  ###   ####    / ##                     █
█              /  ########/    ###  ###  ### ######    ##   ### / ######/  ###                    █ 
█             /     #####       ###  ###  ### ####      ##   ##/   #####    ###                   █
█             |                                                                                   █
█              \)                                                                                 █
█                                                                                                 █
███████████████████████████████████████████████████████████████████████████████████████████████████

//====================================//Definicao e include//===================================/*/

#pragma region INCLUDES

#include <defines.hpp>                        // Definicoes globais
#include <functions.hpp>                      // Funcoes auxiliares
#include <move.hpp>                           // Funcoes de movivmentacao de motores
#include <sensor_task.hpp>                    // Funcoes de sensoreamento
#include <auto_mode.hpp>                      // Logica basica do modo AUTO
#include <strategies.hpp>                     // Estrategias iniciais do modo AUTO
#include <combat.hpp>                         // Loop pos estretegia inicial do modo AUTO
#include <rc_mode.hpp>                        // Interpretacao do controle de PS4 no modo RC

#pragma endregion

//===============================================================================================//
//===========================================//SETUP//===========================================//
//===============================================================================================//

#pragma region SETUPS

//===================================//Inicia a Comunicacao//====================================//

void setup() {
    Serial.begin(115200);                     // Comunicacao serial com baudrate 115200
    
    // Configura os pinos do ESP
    pinMode(TRANSISTOR_SENSOR_PIN, OUTPUT);
    pinMode(SENSOR_IR_F, INPUT);
    pinMode(SENSOR_IR_E, INPUT);
    pinMode(SENSOR_IR_D, INPUT);
    directWriteHigh(TRANSISTOR_SENSOR_PIN);   // Transistor e JSumos ligados

    pinMode(LED_PIN, OUTPUT);
    directWriteLow(LED_PIN);
    pinMode(IR_RECIEVE_PIN, INPUT);

    pinMode(JSUMO_F_PIN, INPUT);
    pinMode(JSUMO_E_PIN, INPUT);
    pinMode(JSUMO_D_PIN, INPUT);

    // LEDs enderecaveis
    setupLeds();                              // Configura os LEDs enderecaveis
    validaSetup(0,0,0,0,0);                   // 5 vermelhos e 0 verdes

    // Comunicacao por IR -> DEVE FICAR NO SETUP
    // Feedback automatico desativado -> Feedback LED configurado manualmente
    IrReceiver.begin(IR_RECIEVE_PIN, DISABLE_LED_FEEDBACK, LED_PIN);
    printActiveIRProtocols(&Serial);
    Serial.println("\n\n//=====//Setup IR feita//=====//");
    validaSetup(1,0,0,0,0);                   // 4 vermelhos e 1 verde

//================================//Criacao da tarefa nos nucleos//==============================//

    setupSensorTask();                        // Configura as tarefas de sensoreamento
    Serial.println("//=====//Setup sensor feita//=====//");
    vTaskDelay(pdMS_TO_TICKS(500));           // Pequeno atraso para protecao
    validaSetup(1,1,0,0,0);                   // 3 vermelhos e 2 verdes

    setupMoveTask();                          // Configura as tarefas de motores
    Serial.println("//=====//Setup motor feita//=====//");
    xTaskNotifyGive(closeServoHandle);
    vTaskDelay(pdMS_TO_TICKS(500));           // Pequeno atraso para protecao
    validaSetup(1,1,1,0,0);                   // 2 vermelhos e 3 verdes

    vTaskDelay(pdMS_TO_TICKS(500));           // Pequeno atraso para protecao
}

#pragma endregion

//===============================================================================================//
//========================================//MODO DE LUTA//=======================================//
//===============================================================================================//

#pragma region MODO DE LUTA

void __init__() {
    Serial.println("//=====//Selecione modo de luta//=====//");

    // Para debug, descomente as secoes abaixo

    //===========//Modo RC//===========//
    /*
    selecionouModo = radioControlado;
    Serial.println("\n//=====//Modo RC selecionado//=====//");
    */

    //==========//Modo AUTO//==========//
    /*
    selecionouModo = autonomo;
    Serial.println("\n//=====//Modo AUTO selecionado//=====//");
    */

    while (selecionouModo == naoSelecionado) {
        if (ultimoComandoIR != 0xFFFF) {
            uint16_t comandoAtual = ultimoComandoIR;
            ultimoComandoIR = 0xFFFF;

            if (comandoAtual == 0x7) {        // Inicia modo AUTO se recebe 8 (0x7) do controle
                blinkLED(8, 25);              // Validacao por LED
                selecionouModo = autonomo;
                Serial.println("\n//=====//Modo AUTO selecionado//=====//");
                validaSetup(1,1,1,1,1);       // 1 vermelho e 4 verdes
                continue;                     // Continua para a proxima iteracao
            }
            
            if (comandoAtual == 0x8) {        // Inicia modo RC se recebe 9 (0x8) do controle
                blinkLED(6, 40);              // Validacao por LED
                selecionouModo = radioControlado;
                Serial.println("\n//=====//Modo RC selecionado//=====//");
                validaSetup(1,1,1,1,1);       // 1 vermelho e 4 verdes
                continue;                     // Continua para a proxima iteracao
            }
        }
    }
}

#pragma endregion

//===============================================================================================//
//============================================//LOOP//===========================================//
//===============================================================================================//

/* 
!INFO | LOOP no ESP32
-----------------------------------
A importancia de se utilizar o loop no codigo, pelo que observamos, se da pelas alteracoes nas 
tarefas de manutencao do proprio ESP e, assim, permitem certas funcionalidades.

A mais importante observada ate o momento foi a capacidade do robo de se movimentar em baixas 
velocidades por conta de uma geracao precisa de sinais PWM em baixa energia
*/

#pragma region LOOP

void loop() {
    if (!inicializado) {                      // Configuracao dos modos de luta
        __init__();
        if (selecionouModo == autonomo) modoAUTO();
        else modoRC();
    } 

    vTaskSuspend(NULL);                       // Suspende o loop pra nao consumir CPU
}

#pragma endregion

//===============================================================================================//