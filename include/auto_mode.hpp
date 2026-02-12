//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef AUTO_MODE_H
#define AUTO_MODE_H

#include <defines.hpp>                        // Definicoes globais
#include <functions.hpp>                      // Funcoes auxiliares
#include <sensor_task.hpp>                    // Funcoes de sensoreamento
#include <strategies.hpp>                     // Estrategias iniciais do modo AUTO

#pragma endregion

//===============================================================================================//
//====================================//INTERPRETA BLUETOOTH//===================================//
//===============================================================================================//

#pragma region INTERPRETA BT

void translateBT() {
    while (true) {
        char receivedChar;
        // Atualiza os LEDs com o status dos sensores
        indicarSensores(valueJsumoE, valueJsumoF, valueJsumoD);

        if (xQueueReceive(btQueue, &receivedChar, pdMS_TO_TICKS(10))) {
            if (isConfiguring) {              // Se configurando estrategia personalizada

                // Se receber o caractere 'enter' finaliza a string
                if (receivedChar == '\n') {
                    btBuffer[bufferIndex] = '\0';

                    // Se receber '.' finaliza a configuracao
                    if (strcmp(btBuffer, ".") == 0) {
                        isConfiguring = false;
                        SerialBT.println("Configuracao finalizada.");
                        Serial.println("Configuracao finalizada.");

                        // Passo final da estrategia: (0, 0, 0) 
                        // Estrutura necessaria para o processamento da sequencia
                        customStrategy[customStrategyCount].speedLeft = 0;
                        customStrategy[customStrategyCount].speedRight = 0;
                        customStrategy[customStrategyCount].delayMs = 0;
                    
                    // Se nao for o ultimo passo configurado, interpretar passo
                    } else {
                        parseAndStoreStep(btBuffer);
                    }
                    bufferIndex = 0;          // Limpa o buffer
                
                // Se exceder o buffer
                } else if (receivedChar >= 32) {
                    if (bufferIndex < BT_BUFFER_SIZE - 1) {
                        btBuffer[bufferIndex++] = receivedChar;
                    } else {
                        Serial.println("Erro: Buffer cheio!");
                        bufferIndex = 0;
                    }
                }

            // Se nao estiver configurando uma estrategia personalizada
            } else {

                // Se receber o caractere '0' entende que a selecao acabou
                if (receivedChar == '0') {    // Fim dos comandos BT
                    Serial.println("//=====//FIM DOS COMANDOS//=====//");
                    SerialBT.println("//=====//FIM DOS COMANDOS//=====//");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    break;                    // Retorna para a modoAUTO
                
                // Se receber o caractere '0' entra no modo de configuracao personalizado   
                } else if (receivedChar == 'z') {
                    isConfiguring = true;
                    customStrategyCount = 0;
                    bufferIndex = 0;
                    SerialBT.println("Modo de configuracao iniciado");
                    SerialBT.println("Envie passo (vel_esq,vel_dir,delay) ou '.' para finalizar");
                    Serial.println("Modo de configuracao iniciado");

                // Se receber qualquer outro caractere, interpreta a informacao
                } else {
                    definicoesBaseBT(receivedChar);

                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));         // Delay para o FreeRTOS
    }
}

#pragma endregion

//===============================================================================================//
//===================================//ESCOLHA DA ESTRATEGIA//===================================//
//===============================================================================================//

#pragma region ESCOLHA ESTRATEGIA

void modoAUTO() {
    setupStratTask();                         // Aloca a funcao da estrategia de luta

    // Cria a fila BT para armazenar as informacoes (evita perdas na comunicacao)
    btQueue = xQueueCreate(BT_QUEUE_LENGTH, sizeof(char));

    Serial.println("//=====//Setup AUTO feita//=====//");
    SerialBT.register_callback(bt_callback);  // Funcao de callback para os dados

    vTaskDelay(pdMS_TO_TICKS(500));           // Pequeno atraso

    SerialBT.begin("Smoker");                 // !INFO | SMOKER, O MAIOR!
    Serial.printf("Bluetooth iniciado. Tentando conectar");
    validaSetup(1,1,1,1,1);                   // 0 vermelhos e 5 verdes
    vTaskDelay(pdMS_TO_TICKS(500));           // Pequeno atraso

    // Espera a conexao BT para continuar
    while (!SerialBT.hasClient()) {
        Serial.printf(".");
        vTaskDelay(pdMS_TO_TICKS(100)); // Espera em pequenos incrementos
    }
    Serial.println();
    Serial.println("Cliente Bluetooth conectado! Iniciando comunicacao");
    SerialBT.println("//=====//Bluetooth conectado!//=====//");
    printCommands();                          // Indica os comandos disponiveis
    lerSensores();                            // Indica quais sensores estao vendo o adversario

    translateBT();                            // Interpreta o char recebido por BT
    inicializado = true;                      // Indica que a configuracao acabou e o loop reinicia
    xTaskNotifyGive(IRCommandHandle);         // Notifica a tarefa de interpretacao do IR
}

#pragma endregion

//===============================================================================================//
//=====================================//FINALIZA O ARQUIVO//====================================//
//===============================================================================================//

#endif