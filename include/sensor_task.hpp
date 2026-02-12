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
        valueJsumoF = !(GPIO.in1.val >> (JSUMO_F_PIN - 32)) & 0x1;
        valueJsumoD = !(GPIO.in1.val >> (JSUMO_D_PIN - 32)) & 0x1;
        valueJsumoE = !(GPIO.in1.val >> (JSUMO_E_PIN - 32)) & 0x1;
        portEXIT_CRITICAL_ISR(&sensorMux);    // Fim da estrutura critica
    } else {
        portENTER_CRITICAL_ISR(&sensorMux);   // Indica estrutura critica: prioridade de execucao
        valueJsumoF = !((GPIO.in >> SENSOR_IR_F) & 0x1);
        valueJsumoD = !((GPIO.in >> SENSOR_IR_D) & 0x1);
        valueJsumoE = !((GPIO.in1.val >> (SENSOR_IR_E - 32)) & 0x1);
        portEXIT_CRITICAL_ISR(&sensorMux);    // Fim da estrutura critica
    }

    if (valueJsumoE) ultimoLado = vistoEsquerda;
    else if (valueJsumoD) ultimoLado = vistoDireita;

    // Variavel para verificar se uma tarefa de maior prioridade foi despertada
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Notifica a SensorTask
    vTaskNotifyGiveFromISR(combatLogicHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {           // Se uma tarefa de maior prioridade foi despertada
        portYIELD_FROM_ISR();                 // Forca desligamento do robo (prioridade maior)
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
        if ((valueJsumoF + valueJsumoE + valueJsumoD >= 4) ||  
            (avancosIteracao > maxAvancosIteracao)         || 
            (numIteracoes > maxIteracoesW)                 ||
            (numIteracoes > maxIteracoesS)                 ||
            (millis() - tempoCombat > 5000)
        ) {
            if(modoFurtivo) xTaskNotifyGive(swSensorHandle);
            modoLuta = ataque;                // Entra no modo de Ataque
        }

//=======================================//Seleciona Modo//======================================//

        switch (modoLuta) {
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
            case slowsearch:
                slowSearch();                 // FSM | Busca em passos mais longos
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
    timerAlarmWrite(sensorTimer, 500, true);  // Definir tempo (µs) aqui
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