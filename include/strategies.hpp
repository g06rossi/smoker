//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef STRATEGIES_H
#define STRATEGIES_H

#include <defines.hpp>                        // Definicoes globais
#include <functions.hpp>                      // Funcoes auxiliares
#include <move.hpp>                           // Funcoes de movivmentacao de motores

#pragma endregion

//===============================================================================================//
//==================================//ESTRATEGIAS SEQUENCIAIS//==================================//
//===============================================================================================//

#pragma region STRATS SEQUENCIAIS

/*
!INFO | Estrutura das estrategias sequenciais
-----------------------------------
A estrutura das estrategias sequenciais deve seguir o formato
const StrategyStep nome[] = {
    {   a,    b,    c}, 
    {   d,    e,    f}, 
    [...]
    {   0,    0,    0}                        // Indica que a sequencia acabou
};

*/

#pragma endregion

//==========================================//FRENTAO//==========================================//

#pragma region FRENTAO

// Vai para frente (3/4 dohyo) em alta velocidade
const StrategyStep frentao[] = {
    {  48,   48,    4}, 
    {  60,   60,    4}, 
    {  72,   72,    4}, 
    {  84,   84,    4}, 
    {  96,   96,    4},
    { 128,  128,    8}, 
    { 160,  160,    8}, 
    { 196,  196,    8}, 
    { 228,  228,   12}, 
    { 255,  255,  108},
    {   0,    0,    0}
};

#pragma endregion

//======================================//FRENTE SEM DELAY//=====================================//

#pragma region FRENTE SEM DELAY

// Vai para frente (3/4 dohyo) em alta velocidade e sem curva de aceleracao
const StrategyStep frenteSemDelay[] = {
    { 255,  255,  200},
    {   0,    0,    0}
};

#pragma endregion

//=========================================//FRENTINHO//=========================================//

#pragma region FRENTINHO

// Vai para frente (1/4 dohyo) em baixa velocidade e sem curva de aceleracao
const StrategyStep frentinho[] = {
    { 127,  127,  200},
    {   0,    0,    0}
};

#pragma endregion

//===========================================//COSTAS//==========================================//

#pragma region COSTAS

// Costas reto
const StrategyStep costasReto[] = {
    { -90,  -90,  200},
    {   0,    0,    0}
};

// Costas para a esquerda
const StrategyStep costasEsquerda[] = {
    { 255, -255,   40}, 
    { -90,  -90,  200},
    {   0,    0,    0}
};

// Costas para a direita
const StrategyStep costasDireita[] = {
    {-255,  255,   40}, 
    { -90,  -90,  200},
    {   0,    0,    0}
};

#pragma endregion

//===========================================//CURVAO//===========================================//

#pragma region CURVAO

// Curva aberta (3/4 dohyo)
const StrategyStep curvaoEsquerda[] = {
    {-255,  255,   70}, 
    { 255,  70,  200},
    { 255,  60,  200},
    { 255, -255, 60},
    {   0,    0,    0}
};

const StrategyStep curvaoDireita[] = {
    { 255, -255,   70}, 
    { 70,  255,  200},
    { 60,  255,  200},
    { -255, 255, 60},
    {   0,    0,    0}
};

#pragma endregion

//==========================================//CURVINHA//=========================================//

#pragma region CURVINHA

// Curva fechada (3/4 dohyo)
const StrategyStep curvinhaEsquerda[] = {
    {-255,  255,  40}, 
    { 255,  90,  400},
    {   0,    0,   0}
};

const StrategyStep curvinhaDireita[] = {
    { 255,-255,  40}, 
    { 90,  255, 200},
    { 60,  255, 200},
    {  0,    0,   0}
};

#pragma endregion

//=========================================//DESVIADA//==========================================//

#pragma region DESVIADA

// Esquerda
const StrategyStep desviadaEsquerda[] = {
    {-255,  255,   75}, 
    { 255,  100,  144},
    { 255, -255,  156},
    { 127,  127,   75},
    {   0,    0,    0}
};

// Direita
const StrategyStep desviadaDireita[] = {
    { 255, -255,   75}, 
    { 127,  255,  144},
    {-255,  255,  156},
    { 127,  127,   75},
    {   0,    0,    0}
};

#pragma endregion

//============================================//EM V//===========================================//

#pragma region EM V

// Movimentacao em V (3/4 dohyo)
const StrategyStep emVEsquerda[] = {
    {-255,  255,   50}, 
    { 255,  255,  150},
    { 255, -255,  145},
    { 255,  255,  145},
    {   0,    0,    0}
};

const StrategyStep emVDireita[] = {
    { 255, -255,   50}, 
    { 255,  255,  155},
    {-255,  255,  155},
    { 255,  255,  125},
    {   0,    0,    0}
};

#pragma endregion

//=========================================//EM VZINHO//=========================================//

#pragma region EM VZINHO

// Movimentacao em V (3/4 dohyo)
const StrategyStep vzinhoEsquerda[] = {
    { 255, 255, 205 },
    { 255, -255, 180},
    { 255, 255, 175 },
    {   0,   0 ,  0 }
};

const StrategyStep vzinhoDireita[] = {
    { 255, 255, 205},
    { -255, 255, 180},
    { 255, 255, 175 },
    {   0,   0 ,  0 }
};

#pragma endregion

//=========================================//EM VZAO//=========================================//

#pragma region EM VZAO

// Movimentacao em V (3/4 dohyo)
const StrategyStep vzaoEsquerda[] = {
    {-255, 255,  90},
    { 255, 255, 155},
    { 255,-255, 177},
    { 255, 255, 125},
    {   0,   0,   0}
};

const StrategyStep vzaoDireita[] = {
    { 255,-255,  90},
    { 255, 255, 200},
    {-255, 255, 125},
    { 255, 255, 125},
    {   0,   0,   0}
};

#pragma endregion

//============================================//GIRO//===========================================//

#pragma region GIRO

/*
!INFO | Estrategia de giro
-----------------------------------
Faz um giro de 180° comecando de costas
Normalmente exigido para round de desempate
*/

const StrategyStep giroEsquerda[] = {
    {-255, 255, 150},
    {   0,   0,   0}
};

const StrategyStep giroDireita[] = {
    { 255,-255, 150},
    {   0,   0,   0}
};

#pragma endregion

//===============================================================================================//
//====================================//EXECUTAR ESTRATEGIA//====================================//
//===============================================================================================//

#pragma region EXECUTAR

// Executa as estrategias sequenciais e personalizadas
void executarEstrategia(const StrategyStep strategySequence[]) {
    // Lida com haste quando necessario
    if (hasteAbaixada) xTaskNotifyGive(openServoHandle);
    xTaskNotifyGive(swSensorHandle);          // Entra no modo furtivo

    // Loop de passos -> sai do loop quando o delay for igual a 0
    for (int i = 0; strategySequence[i].delayMs > 0; ++i) {
        moverMotores(strategySequence[i].speedLeft, strategySequence[i].speedRight);
        vTaskDelay(pdMS_TO_TICKS(strategySequence[i].delayMs));
    }

    if(!modoFurtivo) xTaskNotifyGive(swSensorHandle);          // Sai do modo furtivo
    // Para o robo ao final da execucao
    moverMotores(0, 0);
}

#pragma endregion

//===============================================================================================//
//====================================//TESTES SENSOR MOTOR//====================================//
//===============================================================================================//

// Teste de sensor
void testSensors() {
    for(;;) {
        SerialBT.printf("SENSOR -> E: %d, F: %d, D: %d\n", valueJsumoE, valueJsumoF, valueJsumoD);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Teste de motor
void testMotors() {
    for(;;) {
        moverMotores(255, 255);
        SerialBT.println("FRENTE");
        Serial.println("FRENTE");
        vTaskDelay(pdMS_TO_TICKS(1000));

        moverMotores(-255, 255);
        SerialBT.println("ESQUERDA");
        Serial.println("ESQUERDA");
        vTaskDelay(pdMS_TO_TICKS(1000));

        moverMotores(255, -255);
        SerialBT.println("DIREITA");
        Serial.println("DIREITA");
        vTaskDelay(pdMS_TO_TICKS(1000));

        moverMotores(-255, -255);
        SerialBT.println("TRAS");
        Serial.println("TRAS");
        vTaskDelay(pdMS_TO_TICKS(1000));

        moverMotores(0, 0);
        SerialBT.println("PARADO");
        Serial.println("PARADO");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

//===============================================================================================//
//===================================//SELECIONAR ESTRATEGIA//===================================//
//===============================================================================================//

#pragma region SELECIONAR

void estrategiaLutaBT(char comando) {
    switch(strategy) {

//==================================//Estrategias Sequenciais//==================================//

        // Frentao
        case 'f':
            Serial.println("//=====//FRENTAO INICIADO//=====//");
            executarEstrategia(frentao);
            break;

        // Frente sem delay
        case 'k':
            Serial.println("//=====//FRENTE SEM DELAY INICIADA//=====//");
            executarEstrategia(frenteSemDelay);
            break;

        // Frentinho
        case 'i':
            SerialBT.println("//=====//FRENTINHO INICIADO//=====//");
            executarEstrategia(frentinho);
            break;

        // Costas
        case 'b':
            SerialBT.println("//=====//COSTAS INICIADO//=====//");
            if (direction == reto) executarEstrategia(costasReto);
            else if (direction == esquerda) executarEstrategia(costasEsquerda);
            else if (direction == direita) executarEstrategia(costasDireita);
            break;

        // Curvao
        case 'c':
            SerialBT.println("//=====//CURVAO INICIADO//=====//");
            if (direction == esquerda) executarEstrategia(curvaoEsquerda);
            else executarEstrategia(curvaoDireita);
            break;

        // Curvinha
        case 'u':
            SerialBT.println("//=====//CURVINHA INICIADA//=====//");
            if (direction == esquerda) executarEstrategia(curvinhaEsquerda);
            else executarEstrategia(curvinhaDireita);
            break;

        // Desviada
        case 'd':
            SerialBT.println("//=====//DESVIADA INICIADA//=====//");
            if (direction == esquerda) executarEstrategia(desviadaEsquerda);
            else executarEstrategia(desviadaDireita);
            break;

        // Em V
        case 'v':                             // Movimento em forma de V
            SerialBT.println("//=====//EM V INICIADA//=====//");
            if (direction == esquerda) executarEstrategia(emVEsquerda);
            else executarEstrategia(emVDireita);
            break;

        // Em Vzinho
        case 'n':                             // Movimento em forma de V
            SerialBT.println("//=====//VZINHO INICIADA//=====//");
            if (direction == esquerda) executarEstrategia(vzinhoEsquerda);
            else executarEstrategia(vzinhoDireita);
            break;

            // Em Vzao
        case 'a':                             // Movimento em forma de V
            SerialBT.println("//=====//VZAO INICIADA//=====//");
            if (direction == esquerda) executarEstrategia(vzaoEsquerda);
            else executarEstrategia(vzaoDireita);
            break;

        // Costas: giro de 180°
        case 'g':
            SerialBT.println("//=====//GIRO (180 GRAUS) INICIADO//=====//");
            if (direction == esquerda) executarEstrategia(giroEsquerda);
            else executarEstrategia(giroDireita);
            break;
 
//======================================//Casos especiais//======================================//

        // Estrategia personalizada
        case 'y':
            SerialBT.println("//=====//ESTRATEGIA PERSONALIZADA INICIADA//=====//");
            executarEstrategia(customStrategy);
            break;

        // Iterativo puro (inicia somente modo iterativo)
        case 'p':
            SerialBT.println("//=====//ITERATIVO PURO INICIADO//=====//");
            if (hasteAbaixada) xTaskNotifyGive(openServoHandle);
            if (modoFurtivo) xTaskNotifyGive(swSensorHandle);
            moverMotores(0, 0);
            break;

        case 's':
            SerialBT.println("//=====//TESTE SENSOR INICIADO//=====//");
            if (modoFurtivo) xTaskNotifyGive(swSensorHandle);
            testSensors();
            break;
        case 'm':
            SerialBT.println("//=====//TESTE MOTOR INICIADO//=====//");
            testMotors();
            break;

        // Caso padrao para nao ficar sem fazer nada se o caractere enviado for invalido
        default:
            SerialBT.println("//=====//ESTRATEGIA INVALIDA: INICIANDO DEFENSIVO//=====//");
            if (hasteAbaixada) xTaskNotifyGive(openServoHandle);
            if (modoFurtivo) xTaskNotifyGive(swSensorHandle);
            moverMotores(0, 0);
            break;
    }
}

#pragma endregion
 
//===============================================================================================//
//===================================//CONFIG ESTRATEGIA NOVA//==================================//
//===============================================================================================//

#pragma region PERSONALIZACAO

// Interpreta e armazena o passo da estrategia personalizada
void parseAndStoreStep(const char* buffer) {
    int vel_esq, vel_dir, delay_ms;           // Variaveis para armazenar os valores parseados

    // Usa sscanf para parsear os tres inteiros separados por virgula
    int num_parsed = sscanf(buffer, "%d,%d,%d", &vel_esq, &vel_dir, &delay_ms);

    if (num_parsed != 3) {                    // Se nao tem 3 valores, o formato esta incorreto
        SerialBT.println("Formato invalido! Use: vel_esq,vel_dir,delay");
        Serial.println("Formato invalido! Use: vel_esq,vel_dir,delay");
        return;
    }

    if (customStrategyCount < (MAX_STEPS - 1)) {    // Verifica se ha espaco disponivel no array
        customStrategy[customStrategyCount].speedLeft = vel_esq;
        customStrategy[customStrategyCount].speedRight = vel_dir;
        customStrategy[customStrategyCount].delayMs = delay_ms;
        customStrategyCount++;                // Incrementa o contador de passos
        
        Serial.printf("Passo adicionado: E:%d, D:%d, Delay:%dms\n", vel_esq, vel_dir, delay_ms);
        SerialBT.printf("Passo adicionado: E:%d, D:%d, Delay:%dms\n", vel_esq, vel_dir, delay_ms);
    } else {
        SerialBT.println("Limite de passos da estratégia atingido!");
        Serial.println("Limite de passos da estratégia atingido!");
    }
}

#pragma endregion
 
//===============================================================================================//
//====================================//TASK DA ESTRATEGIA//=====================================//
//===============================================================================================//

#pragma region TASKS

// Inicia a estrategia inicial quando notificado pelo caractere '2' do controle remoto
void estrategiaTask(void *pvParameters) {
    for(;;) {
        // Aguarda ser chamado
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        estrategiaLutaBT(strategy);           // Executa a estratégia
        tempoCombat = millis();
        running = true;                       // Ativa o robo
    }
}

void setupStratTask() {
    xTaskCreatePinnedToCore(
        estrategiaTask,                       // Funcao da tarefa
        "EstrategiaTask",                     // Nome da tarefa
        256 * 32,                             // Tamanho da pilha
        nullptr,                              // Parametros
        14,                                   // Prioridade 14
        &stratLutaHandle,                     // Handle
        APP_CPU_NUM                           // Nucleo 1 onde a tarefa sera executada
    );   
}

#pragma endregion
 
//===============================================================================================//
//=====================================//FINALIZA O ARQUIVO//====================================//
//===============================================================================================//

#endif