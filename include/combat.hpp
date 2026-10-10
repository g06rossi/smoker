//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef COMBAT_H
#define COMBAT_H

#include <defines.hpp>                        // Definicoes globais
#include <move.hpp>                           // Funcoes de movivmentacao de motores

// Declaracoes antecipadas (Forward declarations)
void modoBuscaLinha();
void modoRetorno();

#pragma endregion

//===============================================================================================//
//====================================//PARAMETROS DE AJUSTE//===================================//
//===============================================================================================//

#pragma region PARAMETROS DE AJUSTE

// Velocidades em PWM (0-255) e tempos em ms

// Sensor de linha
const int LINHA_TRESHOLD                  = 3800;  // ADC abaixo disso = linha branca

// Combate
const unsigned long ataqueForcadoMs       = 4000;  // Apos esse tempo de combate entra de vez no Ataque

// Leitura do adversario (giros iguais em todos os modos)
const int velLadoAtual                    = 132;   // Giro para o lado em que o adversario esta
const int velFrenteRapida                 = 214;   // Avanco rapido
const int velFrenteLenta                  = 72;    // Avanco lento
const int acrescimo                       = 32;    // Acrescimo de velocidade para um dos motores

// Modo Busca
const unsigned long buscaAvancoMs         = 60;    // Avanco rapido de cada passo
const unsigned long buscaParadaMs         = 1000;  // Parado entre os avancos
const int maxAvancosIteracao              = 4;     // Limite de avancos por iteracao

// Modo Quebrado
const unsigned long sambadaInicialMs      = 85;    // Primeira sambada para a esquerda
const unsigned long sambadaMs             = 170;   // Demais sambadas (direita e esquerda)
const unsigned long quebradoAvancoMs      = 125;   // Avanco lento de cada passo
const unsigned long quebradoParadaMs      = 1000;  // Parado entre os avancos
const int maxLeituraFrente                = 4;     // Avancos com o adversario a frente ate o Ataque

// Woodpecker
const unsigned long woodpeckerAvancoMs    = 125;   // Avanco lento de cada passo
const unsigned long woodpeckerParadaMs    = 1000;  // Parado entre os avancos
const int maxIteracoesW                   = 10;    // WOOD | Maximo de iteracoes
const int maxIteracoesS                   = 6;     // SLOW | Maximo de iteracoes

// Curva Search: {roda esquerda, roda direita} de cada passo, para cada Direcao da macro
const int curvaArcoDireita[2]             = {0, 160};     // Arco inicial
const int curvaArcoEsquerda[2]            = {150, 0};
const unsigned long curvaArcoDireitaMs    = 300;
const unsigned long curvaArcoEsquerdaMs   = 350;
const int curvaIdaDireita[2]              = {100, -150};  // Zigue-zague: ida
const int curvaIdaEsquerda[2]             = {-120, 100};
const int curvaVoltaDireita[2]            = {-120, -100}; // Zigue-zague: volta
const int curvaVoltaEsquerda[2]           = {100, -150};
const unsigned long curvaZigueZagueMs     = 280;   // Duracao da ida e da volta

// Busca Linha (nao ve o adversario)
const int velBuscaLinhaCruzeiro           = 70;    // Avanco normal
const int velBuscaLinhaArrancada          = 255;   // Arrancada de desencalhe
const unsigned long arrancadaIntervaloMs  = 4000;  // Sem linha -> dispara arrancada
const unsigned long arrancadaDuracaoMs    = 90;
const int velCargaTotal                   = 255;
const unsigned long cargaTotalMs          = 7000;  // Sem tocar a linha -> carga total ate o fim

// Retorno (linha detectada)
const int velRetornoRecuo                 = 255;
const int velRetornoGiro                  = 255;
const unsigned long retornoRecuoLateralMs = 200;   // Linha vista por 1 sensor
const unsigned long retornoGiroLateralMs  = 160;
const unsigned long retornoRecuoFrontalMs = 250;   // Linha vista pelos 2 sensores
const unsigned long retornoGiroFrontalMs  = 225;

#pragma endregion

//====================================//Variaveis de controle//==================================//

#pragma region VARIAVEIS DE CONTROLE

volatile int numIteracoes      = 0;           // WOOD + SLOW | Iteracao de busca atual

int avancosIteracao            = 0;           // BUSCA | Conta os avancos da iteracao
int avancosTotais              = 0;           // BUSCA | Conta os avancos totais

int leituraFrente              = 0;           // QUEBRADO | Qtde de leituras feitas com o sensorF
bool primeiraSambada           = true;        // QUEBRADO | Primeiro passo da movimentacao frontal

// Passos das FSMs (globais para o Retorno poder reinicia-los)
int passoBusca                 = 0;
unsigned long tempoPassoBusca  = 0;
int passoQuebrado              = 0;
unsigned long tempoPassoQuebrado = 0;
int passoWoodpecker            = 0;
unsigned long tempoPassoWoodpecker = 0;
int passoCurva                 = 0;
unsigned long tempoPassoCurva  = 0;
int passoRetorno               = 0;
unsigned long tempoPassoRetorno = 0;

// Busca Linha e Retorno
unsigned long ultimaLinhaMs    = 0;           // Ultimo Retorno ou arrancada (relogio da arrancada)
unsigned long semLinhaMs       = 0;           // Ultimo Retorno (relogio da carga total)
unsigned long avancoMs         = 0;           // Inicio da arrancada atual
bool avancando                 = false;       // Arrancada em andamento
bool cargaTotal                = false;       // Carga total ate o fim da luta
ModoLuta modoAntesRetorno      = ataque;      // Modo para onde o Retorno volta

// Zera o estado de luta no inicio do combate
void resetCombatState() {
    unsigned long agora = millis();
    ultimaLinhaMs = agora;
    semLinhaMs = agora;
    avancoMs = 0;
    avancando = false;
    cargaTotal = false;
    passoRetorno = 0;
}

// Volta as FSMs de busca ao primeiro passo, que reenvia o comando dos motores
void reiniciarPassos() {
    passoBusca = 0;
    passoQuebrado = 0;
    passoWoodpecker = 0;
    passoCurva = 0;
}

// Nenhum sensor ve o adversario (JSumos, ou IRs no modo furtivo)
bool naoVeAdversario() {
    return !valueJsumoE && !valueJsumoF && !valueJsumoD;
}

#pragma endregion

//===============================================================================================//
//========================================//MODO ATAQUE//========================================//
//===============================================================================================//

#pragma region MODO ATAQUE

void modoAtaque() {
    // Se nao ve o inimigo com os JSumos nem com os IRs
    if (naoVeAdversario()) {
        modoBuscaLinha();
        return;

    // Se o inimigo esta a frente
    } else if (((!modoFurtivo) && (valueJsumoF)) ||
        ((modoFurtivo) && (valueJsumoE + valueJsumoF + valueJsumoD == 3)))
    {
        moverMotores(velFrenteRapida, velFrenteRapida);
        return;

    // Se o inimigo esta a esquerda
    } else if (((!modoFurtivo) && (valueJsumoE && !valueJsumoF && !valueJsumoD)) ||
        ((modoFurtivo) && (!valueJsumoD)))
    {
        moverMotores(-velLadoAtual, velLadoAtual);
        return;

    // Se o inimigo esta a direita
    } else if (((!modoFurtivo) && (valueJsumoD && !valueJsumoF && !valueJsumoE)) ||
        ((modoFurtivo) && (!valueJsumoE)))
    {
        moverMotores(velLadoAtual, -velLadoAtual);
        return;

    } else {                                  // Laterais sem a frente: tambem Busca Linha
        modoBuscaLinha();
        return;
    }
}

#pragma endregion

//===============================================================================================//
//========================================//MODO DEFESA//========================================//
//===============================================================================================//

#pragma region MODO DEFESA

void modoDefesa() {
    // Se nao ve o inimigo com os JSumos nem com os IRs
    if (naoVeAdversario()) {
        modoBuscaLinha();
        return;

    // Se o inimigo esta a frente
    } else if (((!modoFurtivo) && (valueJsumoF)) ||
        ((modoFurtivo) && (valueJsumoE + valueJsumoF + valueJsumoD == 3)))
    {
        moverMotores(velFrenteLenta, velFrenteLenta);
        return;

    // Se o inimigo esta a esquerda
    } else if (((!modoFurtivo) && (valueJsumoE && !valueJsumoF && !valueJsumoD)) ||
        ((modoFurtivo) && (!valueJsumoD)))
    {
        moverMotores(-velLadoAtual, velLadoAtual);
        return;

    // Se o inimigo esta a direita
    } else if (((!modoFurtivo) && (valueJsumoD && !valueJsumoF && !valueJsumoE)) ||
        ((modoFurtivo) && (!valueJsumoE)))
    {
        moverMotores(velLadoAtual, -velLadoAtual);
        return;

    } else {                                  // Laterais sem a frente: tambem Busca Linha
        modoBuscaLinha();
        return;
    }
}

#pragma endregion

//===============================================================================================//
//=========================================//MODO GIRO//=========================================//
//===============================================================================================//

#pragma region MODO GIRO

void modoGiro() {
    // Se nao ve o inimigo com os JSumos nem com os IRs
    if (naoVeAdversario()) {
        modoBuscaLinha();
        return;

    // Se o inimigo esta a frente
    } else if (((!modoFurtivo) && (valueJsumoF)) ||
        ((modoFurtivo) && (valueJsumoE + valueJsumoF + valueJsumoD == 3)))
    {
        moverMotores(0, 0);
        return;

    // Gira para a esquerda
    } else if (((!modoFurtivo) && (valueJsumoE && !valueJsumoF && !valueJsumoD)) ||
        ((modoFurtivo) && (!valueJsumoD)))
    {
        moverMotores(-velLadoAtual, velLadoAtual);
        return;

    // Gira para a direita
    } else if (((!modoFurtivo) && (valueJsumoD && !valueJsumoF && !valueJsumoE)) ||
        ((modoFurtivo) && (!valueJsumoE)))
    {
        moverMotores(velLadoAtual, -velLadoAtual);
        return;

    } else {                                  // Laterais sem a frente: tambem Busca Linha
        modoBuscaLinha();
        return;
    }
}

#pragma endregion

//===============================================================================================//
//========================================//MODO BUSCA//=========================================//
//===============================================================================================//

#pragma region MODO BUSCA

void modoBusca() {
    // Se nao ve o inimigo com os JSumos nem com os IRs
    if (naoVeAdversario()) {
        passoBusca = 0;                       // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        modoBuscaLinha();
        return;

    // Gira para a esquerda
    } else if (((!modoFurtivo) && (valueJsumoE && !valueJsumoF && !valueJsumoD)) ||
        ((modoFurtivo) && (!valueJsumoD)))
    {
        moverMotores(-velLadoAtual, velLadoAtual);
        passoBusca = 0;                       // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        return;

    // Gira para a direita
    } else if (((!modoFurtivo) && (valueJsumoD && !valueJsumoF && !valueJsumoE)) ||
        ((modoFurtivo) && (!valueJsumoE)))
    {
        moverMotores(velLadoAtual, -velLadoAtual);
        passoBusca = 0;                       // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        return;

    } else if (((!modoFurtivo) && (valueJsumoF)) ||
        ((modoFurtivo) && (valueJsumoE + valueJsumoF + valueJsumoD == 3)))
    {
        // Maquina de estados se nao houver curva
        switch (passoBusca) {
            case 0:                           // Passo 1: Avancar
                moverMotores(velFrenteRapida, velFrenteRapida);
                tempoPassoBusca = millis();
                passoBusca = 1;
                break;

            case 1:                           // Passo 2: Esperar o avanco terminar
                if ((millis() - tempoPassoBusca) > buscaAvancoMs) passoBusca = 2;
                break;

            case 2:                           // Passo 3: Parar
                moverMotores(0, 0);
                tempoPassoBusca = millis();
                passoBusca = 3;
                break;

            case 3:                           // Passo 4: Esperar a parada terminar
                if ((millis() - tempoPassoBusca) > buscaParadaMs) {
                    passoBusca = 0;           // Reinicia a sequencia
                    avancosIteracao++;        // Acresce a contagem de avancos da iteracao
                    avancosTotais++;          // Acresce a contagem de avancos totais
                }
                break;
        }

    } else {                                  // Laterais sem a frente: tambem Busca Linha
        passoBusca = 0;                       // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        modoBuscaLinha();
        return;
    }
}

#pragma endregion

//===============================================================================================//
//=======================================//MODO QUEBRADO//=======================================//
//===============================================================================================//

// OBS: O modo quebrado nao esta quebrado!!! Movimentacao em angulos quebrados (zigzag)

#pragma region MODO QUEBRADO

void modoQuebrado() {
    // Se nao ve o inimigo com os JSumos nem com os IRs
    if (naoVeAdversario()) {
        passoQuebrado = 0;                    // Reinicia a logica do modo quebrado
        leituraFrente = 0;                    // Reinicia a contagem de leituras com sensor F
        primeiraSambada = true;               // Indica que voltou para o primeiro movimento
        modoBuscaLinha();
        return;

    // Gira para a esquerda
    } else if (((!modoFurtivo) && (valueJsumoE && !valueJsumoF && !valueJsumoD)) ||
        ((modoFurtivo) && (!valueJsumoD)))
    {
        moverMotores(-velLadoAtual, velLadoAtual);
        passoQuebrado = 0;                    // Reinicia a logica do modo quebrado
        leituraFrente = 0;                    // Reinicia a contagem de leituras com sensor F
        primeiraSambada = true;               // Indica que voltou para o primeiro movimento
        return;

    // Gira para a direita
    } else if (((!modoFurtivo) && (valueJsumoD && !valueJsumoF && !valueJsumoE)) ||
        ((modoFurtivo) && (!valueJsumoE)))
    {
        moverMotores(velLadoAtual, -velLadoAtual);
        passoQuebrado = 0;                    // Reinicia a logica do modo quebrado
        leituraFrente = 0;                    // Reinicia a contagem de leituras com sensor F
        primeiraSambada = true;               // Indica que voltou para o primeiro movimento
        return;

    } else if (((!modoFurtivo) && (valueJsumoF)) ||
        ((modoFurtivo) && (valueJsumoE + valueJsumoF + valueJsumoD == 3)))
    {
        // Maquina de estados se nao houver curva
        switch (passoQuebrado) {
            case 0:                           // Passo 1: Primeira sambada esquerda
                moverMotores(-velFrenteLenta, (velFrenteLenta + acrescimo));
                tempoPassoQuebrado = millis();
                passoQuebrado = 1;
                break;

            case 1:                           // Passo 2: Esperar a primeira sambada esquerda
                if (millis() - tempoPassoQuebrado > sambadaInicialMs) {
                    passoQuebrado = 2;
                }
                break;

            case 2:                           // Passo 3: Sambada direita
                moverMotores((velFrenteLenta + acrescimo), -velFrenteLenta);
                tempoPassoQuebrado = millis();
                passoQuebrado = 3;
                break;

            case 3:                           // Passo 4: Esperar a sambada direita
                if (millis() - tempoPassoQuebrado > sambadaMs) {
                    passoQuebrado = 4;
                }
                break;

            case 4:                           // Passo 5: Sambada esquerda
                moverMotores(-velFrenteLenta, (velFrenteLenta + acrescimo));
                tempoPassoQuebrado = millis();
                passoQuebrado = 5;
                break;

            case 5:                           // Passo 6: Esperar a sambada esquerda
                if (millis() - tempoPassoQuebrado > sambadaMs) {
                    if (leituraFrente < maxLeituraFrente) {
                        passoQuebrado = 6;
                    } else modoLuta = ataque;
                }
                break;

            case 6:                           // Passo 7: Se avancou menos de x vezes, avancar
                if (leituraFrente < maxLeituraFrente) {
                    moverMotores(velFrenteLenta, velFrenteLenta);
                    tempoPassoQuebrado = millis();
                    passoQuebrado = 7;
                } else modoLuta = ataque;     // Entra no modo de ataque
                // Essa logica indica que o adversario esta alinhado, pois curvas zeram a contagem
                break;

            case 7:                           // Passo 8: Tempo de avanco
                if (millis() - tempoPassoQuebrado > quebradoAvancoMs) {
                    leituraFrente++;
                    passoQuebrado = 8;
                }
                break;

            case 8:                           // Passo 9: Parar
                moverMotores(0, 0);
                tempoPassoQuebrado = millis();
                passoQuebrado = 9;
                break;

            case 9:                           // Passo 10: Esperar parado
                if (millis() - tempoPassoQuebrado > quebradoParadaMs) {
                    passoQuebrado = 6;        // Reinicia a sequencia de avanco
                }
                break;
        }

    } else {                                  // Laterais sem a frente: tambem Busca Linha
        passoQuebrado = 0;                    // Reinicia a logica do modo quebrado
        leituraFrente = 0;                    // Reinicia a contagem de leituras com sensor F
        primeiraSambada = true;               // Indica que voltou para o primeiro movimento
        modoBuscaLinha();
        return;
    }
}

#pragma endregion

//===============================================================================================//
//======================================//BUSCAS COMPLEXAS//=====================================//
//===============================================================================================//

#pragma region BUSCAS COMPLEXAS

/*
!INFO | Estrategias complexas
-----------------------------------
Essas estrategias fazem uma movimentacao de busca mais complexa que as sequenciais, por isso tem
funcoes separadas que lidam com as necessidades de velocidades, delays e sensoreamento, uma vez que
tambem se utilizam da informacao dos sensores para sair do modo de luta atual.

Ambas as estrategias atuais foram pensadas para iniciar no limite frontal do Dohyo virado para
frente. Isso dificulta a predicao do adversario das possiveis movimentacoes que o robo vai fazer.

Alem disso, os sensores foram pensados para iniciar ligados e depois desligar para que o adversario
consiga enxergar o robo no inicio da luta, foque nele e assim que isso acontecer, o robo receber
passivamente os sinais do adversario. Acredita-se que isso ira trazer uma vantagem de deteccao

Sobre a movimentacao:

- WOODPECKER: Inicialmente se movimenta diagonalmente para tras e busca em formato de W com passos
maiores para frente. Percorre a leitura em formato de L.

- SLOW SEARCH: Inicialmente se movimenta para tras e busca em formato de W com passos curtos
para frente. Percorre a leitura em formato de V.

Ambas as estrategias foram inspiradas nas interpretacoes de Jose Franco e Michael Granda no TCC
para Engenharia Eletrica pela Universidad Politecnica Salesiana em Guayaquil (2020). Mais
informacoes em: https://tinyurl.com/pt7b33u3
*/

#pragma endregion

//=========================================//WOODPECKER//========================================//

#pragma region WOODPECKER

void buscaWoodpecker() {

/*

    static int passoWoodpecker = 0;
    static unsigned long tempoPassoWoodpecker = 0;

    modoFurtivo = false; xTaskNotifyGive(swSensorHandle);

    switch (passoWoodpecker) {
        case 0:
            moverMotores(200, 200);
            tempoPassoWoodpecker = millis();
            passoWoodpecker = 1;
            break;

        case 1:
            if (millis() - tempoPassoWoodpecker > 75) passoWoodpecker = 2;
            break;

        case 2:
            moverMotores(0, 0);
            tempoPassoWoodpecker = millis();
            passoWoodpecker = 3;
            break;

        case 3:
            if (millis() - tempoPassoWoodpecker > 75) passoWoodpecker = 4;
            break;

        case 4:
            moverMotores(-200, 200);
            tempoPassoWoodpecker = millis();
            passoWoodpecker = 5;
            break;

        case 5:
            if (millis() - tempoPassoWoodpecker > 75) passoWoodpecker = 6;
            break;

        case 6:
            moverMotores(200, -200);
            tempoPassoWoodpecker = millis();
            passoWoodpecker = 7;
            break;

        case 7:
            if (millis() - tempoPassoWoodpecker > 150) passoWoodpecker = 8;
            break;

        case 8:
            moverMotores(0, 0);
            tempoPassoWoodpecker = millis();
            passoWoodpecker = 9;
            break;

        case 9:
            if (millis() - tempoPassoWoodpecker > 150) {
                numIteracoes++;
                passoWoodpecker = 0;
            }
            break;
    }

    // Modos recuados para ter cuidado ja que nao encontrou o adversario no limite de iteracoes
    if ((numIteracoes > maxIteracoesS) && (modoLuta != giro)) modoLuta = defesa;

    // Modo de luta de Giro mantem JSumos desligados
    if (modoFurtivo && (modoLuta != giro)) {
        modoFurtivo = false;
        xTaskNotifyGive(swSensorHandle);
    }

*/

    // Se nao ve o inimigo com os JSumos nem com os IRs
    if (naoVeAdversario()) {
        passoWoodpecker = 0;                  // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        modoBuscaLinha();
        return;

    // Gira para a esquerda
    } else if (((!modoFurtivo) && (valueJsumoE && !valueJsumoF && !valueJsumoD)) ||
        ((modoFurtivo) && (!valueJsumoD)))
    {
        moverMotores(-velLadoAtual, velLadoAtual);
        passoWoodpecker = 0;                  // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        return;

    // Gira para a direita
    } else if (((!modoFurtivo) && (valueJsumoD && !valueJsumoF && !valueJsumoE)) ||
        ((modoFurtivo) && (!valueJsumoE)))
    {
        moverMotores(velLadoAtual, -velLadoAtual);
        passoWoodpecker = 0;                  // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        return;

    } else if (((!modoFurtivo) && (valueJsumoF)) ||
        ((modoFurtivo) && (valueJsumoE + valueJsumoF + valueJsumoD == 3)))
    {
        // Maquina de estados se nao houver curva
        switch (passoWoodpecker) {
            case 0:                           // Passo 1: Avancar
                moverMotores(velFrenteLenta, velFrenteLenta);
                tempoPassoWoodpecker = millis();
                passoWoodpecker = 1;
                break;

            case 1:                           // Passo 2: Esperar o avanco terminar
                if (millis() - tempoPassoWoodpecker > woodpeckerAvancoMs) passoWoodpecker = 2;
                break;

            case 2:                           // Passo 3: Parar
                moverMotores(0, 0);
                tempoPassoWoodpecker = millis();
                passoWoodpecker = 3;
                break;

            case 3:                           // Passo 4: Esperar a parada terminar
                if (millis() - tempoPassoWoodpecker > woodpeckerParadaMs) {
                    passoWoodpecker = 0;      // Reinicia a sequencia
                    avancosIteracao++;        // Acresce a contagem de avancos da iteracao
                    avancosTotais++;          // Acresce a contagem de avancos totais
                }
                break;
        }

    } else {                                  // Laterais sem a frente: tambem Busca Linha
        passoWoodpecker = 0;                  // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        modoBuscaLinha();
        return;
    }
}

#pragma endregion

//========================================//CURVA SEARCH//========================================//

#pragma region CURVA SEARCH

void curvaSearch() {
    switch (passoCurva) {

        case 0:
            if (direction == direita) moverMotores(curvaArcoDireita[0], curvaArcoDireita[1]);
            else moverMotores(curvaArcoEsquerda[0], curvaArcoEsquerda[1]);
            tempoPassoCurva = millis();
            passoCurva = 1;
            break;

        case 1:
            if (direction == direita && millis() - tempoPassoCurva >= curvaArcoDireitaMs) passoCurva = 2;
            else if (direction == esquerda && millis() - tempoPassoCurva >= curvaArcoEsquerdaMs) passoCurva = 2;
            break;

        case 2:
            if (direction == direita) moverMotores(curvaIdaDireita[0], curvaIdaDireita[1]);
            else moverMotores(curvaIdaEsquerda[0], curvaIdaEsquerda[1]);
            tempoPassoCurva = millis();
            passoCurva = 3;
            break;

        case 3:
            if ((millis() - tempoPassoCurva) >= curvaZigueZagueMs) passoCurva = 4;
            break;

        case 4:
            if (direction == direita) moverMotores(curvaVoltaDireita[0], curvaVoltaDireita[1]);
            else moverMotores(curvaVoltaEsquerda[0], curvaVoltaEsquerda[1]);
            tempoPassoCurva = millis();
            passoCurva = 5;
            break;

        case 5:
            if ((millis() - tempoPassoCurva) >= curvaZigueZagueMs) passoCurva = 2;
            break;
    }
}

#pragma endregion

//===============================================================================================//
//======================================//MODO BUSCA LINHA//=====================================//
//===============================================================================================//

#pragma region MODO BUSCA LINHA

// Chamada pelos modos quando nao veem o adversario. Os relogios contam desde o ultimo Retorno
void modoBuscaLinha() {
    unsigned long agora = millis();

    // Etapa 2: Carga Total
    if (!cargaTotal && (agora - semLinhaMs >= cargaTotalMs)) {
        cargaTotal = true;
    }

    if (cargaTotal) {
        moverMotores(velCargaTotal, velCargaTotal);
        return;
    }

    // Etapa 1: Arrancada de desencalhe
    if (!avancando && (agora - ultimaLinhaMs >= arrancadaIntervaloMs)) {
        avancando = true;
        avancoMs = agora;
    }

    if (avancando) {
        if (agora - avancoMs < arrancadaDuracaoMs) {
            moverMotores(velBuscaLinhaArrancada, velBuscaLinhaArrancada);
        } else {
            avancando = false;
            ultimaLinhaMs = agora;
            moverMotores(velBuscaLinhaCruzeiro, velBuscaLinhaCruzeiro);
        }
    } else {
        moverMotores(velBuscaLinhaCruzeiro, velBuscaLinhaCruzeiro);
    }
}

#pragma endregion

//===============================================================================================//
//========================================//MODO RETORNO//=======================================//
//===============================================================================================//

#pragma region MODO RETORNO

void modoRetorno() {
    bool frontal = (viuLinha == linhaAMBAS);
    unsigned long tempoRecuo = frontal ? retornoRecuoFrontalMs : retornoRecuoLateralMs;
    unsigned long tempoGiro  = frontal ? retornoGiroFrontalMs  : retornoGiroLateralMs;

    switch (passoRetorno) {
        case 0:
            moverMotores(-velRetornoRecuo, -velRetornoRecuo);
            tempoPassoRetorno = millis();
            passoRetorno = 1;
            break;

        case 1:
            if (millis() - tempoPassoRetorno >= tempoRecuo) passoRetorno = 2;
            break;

        case 2: {
            // Vira para o lado oposto ao da linha, de volta para dentro da arena
            bool girarParaDireita;
            if (viuLinha == linhaESQ)      girarParaDireita = true;
            else if (viuLinha == linhaDIR) girarParaDireita = false;
            else                           girarParaDireita = (ultimoLado == vistoDireita);

            if (girarParaDireita) moverMotores(velRetornoGiro, -velRetornoGiro);
            else                  moverMotores(-velRetornoGiro, velRetornoGiro);
            tempoPassoRetorno = millis();
            passoRetorno = 3;
            break;
        }

        case 3:
            if (millis() - tempoPassoRetorno >= tempoGiro) {
                passoRetorno = 0;
                ultimaLinhaMs = millis();
                semLinhaMs = millis();
                reiniciarPassos();            // A FSM do modo recomeca e reenvia o comando dos motores
                modoLuta = modoAntesRetorno;
            }
            break;
    }
}

#pragma endregion

//===============================================================================================//
//=====================================//FINALIZA O ARQUIVO//====================================//
//===============================================================================================//

#endif
