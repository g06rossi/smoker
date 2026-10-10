//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include <defines.hpp>                        // Definicoes globais
#include <functions.hpp>                      // Funcoes auxiliares
#include <move.hpp>                           // Funcoes de movivmentacao de motores
#include <combat.hpp>                         // Loop pos estretegia inicial do modo AUTO

bool JSumoLigado = true;

#pragma endregion

//===============================================================================================//
//========================================//PARAR ROBO//=========================================//
//===============================================================================================//

#pragma region PARAR ROBO

void stopRobot(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for(;;) {
        SerialBT.println("PAROU");
        running = false;                      // Para os loops de Defensivo e Ofensivo
        brakeMotors();                        // Trava os motores
        vTaskDelay(pdMS_TO_TICKS(1000));      // Delay para garantir a parada
    }
}

void irMonitorTask(void *pvParameters) {      // Monitora a cada 100 ms se o robo deve parar 
    for (;;) {
        if (IrReceiver.decode()) {
            // Armazena o comando decodificado em uma variavel global
            ultimoComandoIR = IrReceiver.decodedIRData.command;
            // Se recebe IR 3 (0x2), notifica a task de parada
            if (ultimoComandoIR == 0x2) {
                blinkLED(8, 25);
                xTaskNotifyGive(stopRobotHandle);
            }
            IrReceiver.resume();
        }
        vTaskDelay(pdMS_TO_TICKS(100));       // 100 ms para as outras tarefas serem executadas
    }
}

#pragma endregion

//===============================================================================================//
//====================================//LEITURA DOS SENSORES//===================================//
//===============================================================================================//

//======================================//Desliga Sensores//=====================================//

#pragma region DESLIGA SENSOR

void switchSensor(void *pvParameters) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (JSumoLigado) {                    // Desliga o transistor e desconecta GND dos JSumos
            GPIO.out_w1tc = ((uint32_t)1 << TRANSISTOR_SENSOR_PIN);
            JSumoLigado = false;
        } else {                              // Liga o transistor e conecta GND dos JSumos
            GPIO.out_w1ts = ((uint32_t)1 << TRANSISTOR_SENSOR_PIN);
            JSumoLigado = true;
        }
    }
}

#pragma endregion

//======================================//Leitura Sensores//=====================================//

#pragma region LEITURA SENSOR

void IRAM_ATTR readSensors() {
    if(JSumoLigado) {
        portENTER_CRITICAL_ISR(&sensorMux);   // Indica estrutura critica: prioridade de execucao
        valueJsumoF = lerGPIO(JSUMO_F_PIN);
        valueJsumoD = lerGPIO(JSUMO_D_PIN);
        valueJsumoE = lerGPIO(JSUMO_E_PIN);
        portEXIT_CRITICAL_ISR(&sensorMux);    // Fim da estrutura critica
    } else {
        portENTER_CRITICAL_ISR(&sensorMux);   // Indica estrutura critica: prioridade de execucao
        valueJsumoF = !lerGPIO(SENSOR_IR_F);
        valueJsumoD = !lerGPIO(SENSOR_IR_D);
        valueJsumoE = !lerGPIO(SENSOR_IR_E);
        portEXIT_CRITICAL_ISR(&sensorMux);    // Fim da estrutura critica
    }

    if (valueJsumoE) ultimoLado = vistoEsquerda;
    else if (valueJsumoD) ultimoLado = vistoDireita;

    // Variavel para verificar se uma tarefa de maior prioridade foi despertada
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Dispara a leitura da linha e a logica de combate no mesmo ciclo
    vTaskNotifyGiveFromISR(analogSensorHandle, &xHigherPriorityTaskWoken);
    vTaskNotifyGiveFromISR(combatLogicHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {           // Se uma tarefa de maior prioridade foi despertada
        portYIELD_FROM_ISR();                 // Forca desligamento do robo (prioridade maior)
    }
}

#pragma endregion

//=====================================//Sensores Analogicos//===================================//

#pragma region SENSORES ANALOGICOS

/*
!INFO | Linha analogica
-----------------------------------
O ADC nao pode ser lido dentro da ISR, por isso a linha e lida nesta task, que a ISR acorda a cada
ciclo de 300us. ADC abaixo de LINHA_TRESHOLD = linha branca. Os pinos precisam ser do ADC1
(GPIO 32-39)

Nao use analogRead aqui: no core Arduino 2.x ele refaz pinMode e atenuacao a cada chamada e pode
passar de 300us. A task entao ocupa 100% do nucleo 0, a IDLE0 nao roda e o watchdog de tarefas
reinicia o ESP. O ADC e configurado uma vez e lido com adc1_get_raw
*/
void analogSensorTask(void *pvParameters) {
    adc1_channel_t canalLinhaE = (adc1_channel_t)digitalPinToAnalogChannel(LINHA_E_PIN);
    adc1_channel_t canalLinhaD = (adc1_channel_t)digitalPinToAnalogChannel(LINHA_D_PIN);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(canalLinhaE, ADC_ATTEN_DB_12);   // Mesma escala do analogRead padrao
    adc1_config_channel_atten(canalLinhaD, ADC_ATTEN_DB_12);
    adc_power_acquire();                      // ADC sempre ligado (evita glitch nos GPIO 36 e 39)

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   // Acordada pela ISR dos sensores
        int64_t inicioCiclo = esp_timer_get_time();

        valueLinhaE = adc1_get_raw(canalLinhaE) < LINHA_TRESHOLD;
        valueLinhaD = adc1_get_raw(canalLinhaD) < LINHA_TRESHOLD;

        // Protecao: se o ciclo ficou longo demais, cede o nucleo 0 para a IDLE0 alimentar o watchdog
        if (esp_timer_get_time() - inicioCiclo > ADC_CICLO_LIMITE_US) vTaskDelay(pdMS_TO_TICKS(1));
    }
}

#pragma endregion

//================================//Interpreta controle remoto//=================================//

#pragma region INTERPRETA IR

void handleIRCommand(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Aguarda notificacao
    for (;;) {
        if (ultimoComandoIR != 0xFFFF) {      // Verifica se há um comando IR recebido
            uint16_t comandoAtual = ultimoComandoIR;
            ultimoComandoIR = 0xFFFF;

            // Indica estar pronto para AUTO se recebe 1 (0x0) do controle
            if (comandoAtual == 0x0) {
                Serial.println("Pronto");
                SerialBT.println("Pronto");
                ready = true;                 // Pronto para iniciar a movimentacao
                AnnihilationModeLeds();       // LEDs vermelhos para a sede de ser campeao
                blinkLED(1, 25);              // Pisca o LED builtin se recebe IR 1
            }

            // Inicia movimento AUTO se recebe 2 (0x1) do controle
            if (comandoAtual == 0x1 && ready) {
                // Notifica a task de estrategia
                clearLeds();                  // Apaga LEDs enderecaveis
                xTaskNotifyGive(stratLutaHandle);
                vTaskDelete(NULL);            // Encerra esta task
            }
            IrReceiver.resume();              // Limpa o buffer IR
        }
    }
}

#pragma endregion

//===============================================================================================//
//=====================================//LOGICA DE COMBATE//=====================================//
//===============================================================================================//

#pragma region LOGICA DE COMBATE

void combatLogicTask(void *pvParameters) {
    for (;;) {                                // Define a logica de combate que sera ativada
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(!running) continue;                // Trava de seguranca da logica

//================================//Condicoes de mudanca de modo//===============================//

        // GERAL | Se dois ou mais sensores detectam o inimigo, desativa o modo furtivo e ataca
        // BUSCA | Se os avancos (iter || tot) forem suficientes, desativa o modo furtivo e ataca
        // WOOD  | Se os avancos forem suficientes, desativa o modo furtivo e ataca
        // SLOW  | Se os avancos forem suficientes, desativa o modo furtivo e ataca
        if ((millis() - tempoCombat) >= ataqueForcadoMs && modoLuta != retorno) {
            modoLuta = ataque;                // Entra no modo de Ataque
        }

        // RETORNO DE LINHA | Prioridade sobre todos os modos de luta: evita sair do dohyo
        if ((valueLinhaE || valueLinhaD) && modoLuta != retorno) {
            if (valueLinhaE && !valueLinhaD) viuLinha = linhaESQ;
            else if (valueLinhaD && !valueLinhaE) viuLinha = linhaDIR;
            else viuLinha = linhaAMBAS;
            modoAntesRetorno = modoLuta;      // O Retorno volta para o modo em que estava
            modoLuta = retorno;
        }

//=======================================//Seleciona Modo//======================================//

        switch (modoLuta) {
            case retorno:
                modoRetorno();                // FSM | Evita sair do dohyo
                break;
            case ataque:
                modoAtaque();                 // Ataca o adversario de forma rapida
                break;
            case defesa:
                modoDefesa();                 // Ataca o adversario devagar
                break;
            case giro:
                modoGiro();                   // Acompanha o adversario sem andar pra frente
                break;
            case busca:
                modoBusca();                  // FSM | Acompanha o adversario com passos
                break;
            case quebrado:
                modoQuebrado();               // FSM | Acompanha o adversario em ziguezague
                break;
            case woodpecker:
                buscaWoodpecker();            // FSM | Busca em Woodpecker
                break;
            case curvasearch:
                curvaSearch();                 // FSM | Busca em passos mais longos
                break;
            default:
                modoAtaque();
                break;
        }
    }
}

/*
!INFO | FSM (Finite State Machine) Cooperativa Nao-Bloqueante
-----------------------------------
A logica dos modos complexos segue uma Maquina de Estados Finitos (FSM) cooperativa. A transicao 
entre os estados e baseada em tempo ('millis()') ao inves de delays, garantindo que a funcao de 
modo retorne imediatamente a cada tick

Isso mantem a latencia do loop de controle principal proxima de zero, permitindo preempcao 
instantanea da estrategia atual por uma de maior prioridade baseada em novas leituras dos sensores
*/

#pragma endregion

//===============================================================================================//
//===========================================//SETUP//===========================================//
//===============================================================================================//

//===========================================//Timer//===========================================//

#pragma region SETUP TIMER

void startTimer() {
    sensorTimer = timerBegin(0, 80, true);    // Timer de 80 ticks
    timerAttachInterrupt(sensorTimer, &readSensors, true); // Qual funcao sera acordada
    timerAlarmWrite(sensorTimer, 300, true);  // Definir tempo (µs) aqui
    timerAlarmEnable(sensorTimer);            // Ligar o timer
}

#pragma endregion

//======================================//Task de alocacao//=====================================//

#pragma region TASK ALOCACAO

void setupSensorTask() {
    xTaskCreatePinnedToCore(                  // Parar o robo
        stopRobot,                            // Funcao da tarefa
        "StopRobot",                          // Nome da tarefa
        256 * 16,                             // Tamanho da pilha
        nullptr,                              // Parametros
        17,                                   // Prioridade 16
        &stopRobotHandle,                     // Handle
        PRO_CPU_NUM                           // Nucleo 0 onde a tarefa sera executada
    );
    
    xTaskCreatePinnedToCore(                  // Monitorar o sinal IR para parar
        irMonitorTask,                        // Funcao da tarefa
        "IR_Monitor",                         // Nome da tarefa
        256 * 16,                             // Tamanho da pilha
        nullptr,                              // Parametros
        16,                                   // Prioridade 16
        NULL,                                 // Handle
        PRO_CPU_NUM                           // Nucleo 0 onde a tarefa sera executada
    );
    
    xTaskCreatePinnedToCore(                  // Ligar e desligar os sensores
        switchSensor,                         // Funcao da tarefa
        "SwitchSensor",                       // Nome da tarefa
        256 * 16,                             // Tamanho da pilha
        nullptr,                              // Parametros
        15,                                   // Prioridade 15
        &swSensorHandle,                      // Handle
        PRO_CPU_NUM                           // Nucleo 0 onde a tarefa sera executada
    );

    xTaskCreatePinnedToCore(                  // Le a linha (analogica)
        analogSensorTask,                     // Funcao da tarefa
        "AnalogSensor",                       // Nome da tarefa
        256 * 16,                             // Tamanho da pilha
        nullptr,                              // Parametros
        15,                                   // Prioridade 15
        &analogSensorHandle,                  // Handle (a ISR notifica, criar antes do timer)
        PRO_CPU_NUM                           // Nucleo 0 onde a tarefa sera executada
    );

    xTaskCreatePinnedToCore(                  // Monitorar o sinal IR para iniciar a luta
        handleIRCommand,                      // Funcao da tarefa
        "HandleIRCommand",                    // Nome da tarefa
        256 * 16,                             // Tamanho da pilha
        nullptr,                              // Parametros
        14,                                   // Prioridade 14
        &IRCommandHandle,                     // Handle
        APP_CPU_NUM                           // Nucleo 1 onde a tarefa sera executada
    ); 
    
    xTaskCreatePinnedToCore(                  // Lida com a logica de combate
        combatLogicTask,                      // Funcao da tarefa
        "SensorTask",                         // Nome da tarefa
        256 * 32,                             // Tamanho da pilha
        nullptr,                              // Parametros
        14,                                   // Prioridade 14
        &combatLogicHandle,                   // Handle
        APP_CPU_NUM                           // Nucleo 1 onde a tarefa sera executada
    );
    
    startTimer();                             // Configurar o timer dos sensores
}

#pragma endregion

//===============================================================================================//
//=====================================//FINALIZA O ARQUIVO//====================================//
//===============================================================================================//

#endif