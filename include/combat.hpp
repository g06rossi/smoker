//===============================================================================================//
//=====================================//INCLUDES E DEFINES//====================================//
//===============================================================================================//

#pragma region INCLUDES

#ifndef COMBAT_H
#define COMBAT_H

#include <defines.hpp>                        // Definicoes globais
#include <move.hpp>                           // Funcoes de movivmentacao de motores

#define velUltimoLado          64             // Velocidade de giro para o ultimo lado visto
#define velLadoAtual           132            // Velocidade de giro para o lado atual
#define velFrenteRapida        192            // Velocidade de avanco rapido
#define velFrenteLenta         54             // Velocidade de avanco lento
#define acrescimo              32             // Acrescimo de velocidade para um dos motores

#define maxAvancosIteracao     4              // BUSCA | Limite de avancos por iteracao
#define maxLeituraFrente       4              // QUEBRADO | Maximo de leituras com o sensorF
#define maxIteracoesW          10             // WOOD | Maximo de iteracoes 
#define maxIteracoesS          6              // SLOW | Maximo de iteracoes

volatile int numIteracoes      = 0;           // WOOD + SLOW | Iteracao de busca atual

int avancosIteracao            = 0;           // BUSCA | Conta os avancos da iteracao
int avancosTotais              = 0;           // BUSCA | Conta os avancos totais

int leituraFrente              = 0;           // QUEBRADO | Qtde de leituras feitas com o sensorF
bool primeiraSambada           = true;        // QUEBRADO | Primeiro passo da movimentacao frontal

#pragma endregion

//===============================================================================================//
//========================================//MODO ATAQUE//========================================//
//===============================================================================================//

#pragma region MODO ATAQUE

void modoAtaque() {
    if (valueJsumoF) {                        // Se o inimigo esta a frente
        moverMotores(velFrenteRapida, velFrenteRapida);
        return;
    
    // Se o inimigo esta a esquerda
    } else if (valueJsumoE && !valueJsumoF && !valueJsumoD) {
        moverMotores(-velLadoAtual, velLadoAtual);
        return;
    
    // Se o inimigo esta a direita
    } else if (valueJsumoD && !valueJsumoF && !valueJsumoE) {
        moverMotores(velLadoAtual, -velLadoAtual);
        return;

    } else {
        switch (ultimoLado) {             // Procura o inimigo no ultimo lado visto
            case vistoEsquerda:
                moverMotores(-velUltimoLado, velUltimoLado);
                return;
               
            case vistoDireita:
                moverMotores(velUltimoLado, -velUltimoLado);
                return;
              
            default:                      // Se nunca viu, gira para tentar encontrar
                moverMotores(velUltimoLado, -velUltimoLado);
                return;
        }
    }
}

#pragma endregion

//===============================================================================================//
//========================================//MODO DEFESA//========================================//
//===============================================================================================//

#pragma region MODO DEFESA

void modoDefesa() {
    if (valueJsumoF) {                         // Se o inimigo esta a frente
        moverMotores(velFrenteLenta, velFrenteLenta);
        return;

    // Se o inimigo esta a esquerda
    } else if (valueJsumoE && !valueJsumoF && !valueJsumoD) {
        moverMotores(-velLadoAtual, velLadoAtual);
        return;

    // Se o inimigo esta a direita
    } else if (valueJsumoD && !valueJsumoF && !valueJsumoE) {
        moverMotores(velLadoAtual, -velLadoAtual);
        return;

    } else {
        switch (ultimoLado) {             // Procura o inimigo no ultimo lado visto
            case vistoEsquerda:
                moverMotores(-velUltimoLado, velUltimoLado);
                return;
               
            case vistoDireita:
                moverMotores(velUltimoLado, -velUltimoLado);
                return;
              
            default:                      // Se nunca viu, gira para tentar encontrar
                moverMotores(velUltimoLado, -velUltimoLado);
                return;
        }
    }
}

#pragma endregion

//===============================================================================================//
//=========================================//MODO GIRO//=========================================//
//===============================================================================================//

#pragma region MODO GIRO

void modoGiro() {
    if (valueJsumoF) {                        // Se ve oponente a frente, para
        moverMotores(0, 0);
        return;

    // Gira para a esquerda
    } else if (valueJsumoE && !valueJsumoF && !valueJsumoD) {
        moverMotores(-velLadoAtual, velLadoAtual);
        return;

    // Gira para a direita
    } else if (valueJsumoD && !valueJsumoF && !valueJsumoE) {
        moverMotores(velLadoAtual, -velLadoAtual);
        return;

    } else {
        switch (ultimoLado) {             // Procura o inimigo no ultimo lado visto
            case vistoEsquerda:
                moverMotores(-velUltimoLado, velUltimoLado);
                return;
               
            case vistoDireita:
                moverMotores(velUltimoLado, -velUltimoLado);
                return;
              
            default:                      // Se nunca viu, gira para tentar encontrar
                moverMotores(-velUltimoLado, velUltimoLado);
                return;
        }
    }
}

#pragma endregion

//===============================================================================================//
//========================================//MODO BUSCA//=========================================//
//===============================================================================================//

#pragma region MODO BUSCA

void modoBusca() {
    static int passoBusca = 0;
    static unsigned long tempoPassoBusca = 0;

    // Logica de curva executada a cada pulso
    if (valueJsumoE) {
        moverMotores(-velLadoAtual, velLadoAtual);
        passoBusca = 0;                      // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        return;

    } else if (valueJsumoD) {
        moverMotores(velLadoAtual, -velLadoAtual);
        passoBusca = 0;                      // Reinicia a logica de busca
        avancosIteracao = 0;                  // Reinicia a contagem de avancos da iteracao
        return;

    } else if (valueJsumoF && !valueJsumoE && !valueJsumoD) {
        // Maquina de estados se nao houver curva
        switch (passoBusca) {
            case 0:                           // Passo 1: Avancar por 50ms
                moverMotores(velFrenteLenta, velFrenteLenta);
                tempoPassoBusca = millis();
                passoBusca = 1;
                break;

            case 1:                           // Passo 2: Esperar os 50ms terminarem
                if (millis() - tempoPassoBusca > 125) passoBusca = 2;
                break;

            case 2:                           // Passo 3: Parar por 500ms
                moverMotores(0, 0);
                tempoPassoBusca = millis();
                passoBusca = 3;
                break;

            case 3:                           // Passo 4: Esperar os 500ms terminarem
                if (millis() - tempoPassoBusca > 1000) {
                    passoBusca = 0;          // Reinicia a sequencia
                    avancosIteracao++;        // Acresce a contagem de avancos da iteracao
                    avancosTotais++;          // Acresce a contagem de avancos totais
                }
                break;
        }

    } else {                                   // Se nao ve o inimigo
        switch (ultimoLado) {
            case vistoDireita:                 // Procura o inimigo no ultimo lado visto
                moverMotores(velUltimoLado, -velUltimoLado);
                passoBusca = 0;               // Reinicia a logica de busca
                avancosIteracao = 0;           // Reinicia a contagem de avancos da iteracao
                break;
            
            default:                           // Caso nao veja na direita, gira para a esquerda
                moverMotores(velUltimoLado, -velUltimoLado);
                passoBusca = 0;               // Reinicia a logica de busca
                avancosIteracao = 0;           // Reinicia a contagem de avancos da iteracao
                break;
        }
    }
}

#pragma endregion

//===============================================================================================//
//=======================================//MODO QUEBRADO//=======================================//
//===============================================================================================//

// OBS: O modo quebrado nao esta quebrado!!! Movimentacao em angulos quebrados (zigzag)

#pragma region MODO QUEBRADO

void modoQuebrado() {
    static int passoQuebrado = 0;
    static unsigned long tempoPassoQuebrado = 0;

    // Logica de curva executada a cada pulso
    if (valueJsumoE && !valueJsumoD) {
        moverMotores(-velLadoAtual, velLadoAtual);
        passoQuebrado = 0;                   // Reinicia a logica do modo quebrado
        leituraFrente = 0;                    // Reinicia a contagem de leituras com sensor F
        primeiraSambada = true;               // Indica que voltou para o primeiro movimento
        return;

    } else if (valueJsumoD && !valueJsumoE) {
        moverMotores(velLadoAtual, -velLadoAtual);
        passoQuebrado = 0;                   // Reinicia a logica do modo quebrado
        leituraFrente = 0;                    // Reinicia a contagem de leituras com sensor F
        primeiraSambada = true;               // Indica que voltou para o primeiro movimento
        return;

    } else if (valueJsumoF && !valueJsumoD && !valueJsumoE) {
    // Maquina de estados se nao houver curva
        switch (passoQuebrado) {
            case 0:                           // Passo 1: Primeira sambada esquerda
                moverMotores(-velFrenteLenta, (velFrenteLenta + acrescimo));
                tempoPassoQuebrado = millis();
                passoQuebrado = 1;
                break;

            case 1:                           // Passo 2: Esperar a primeira sambada esquerda
                if (millis() - tempoPassoQuebrado > 85) {
                    passoQuebrado = 2;
                }
                break;

            case 2:                           // Passo 3: Sambada direita
                moverMotores((velFrenteLenta + acrescimo), -velFrenteLenta); 
                tempoPassoQuebrado = millis();
                passoQuebrado = 3;
                break;

            case 3:                           // Passo 4: Esperar a sambada direita
                if (millis() - tempoPassoQuebrado > 170) {
                    passoQuebrado = 4;
                }
                break;

            case 4:                           // Passo 5: Sambada esquerda     
                moverMotores(-velFrenteLenta, (velFrenteLenta + acrescimo));
                tempoPassoQuebrado = millis();
                passoQuebrado = 5;
                break;

            case 5:                           // Passo 6: Esperar a sambada esquerda
                if (millis() - tempoPassoQuebrado > 170) {
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
                if (millis() - tempoPassoQuebrado > 125) {
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
                if (millis() - tempoPassoQuebrado > 1000) {
                    passoQuebrado = 6;       // Reinicia a sequencia de avanco
                }
                break;
        }

    } else {                                   // Se nao ve o inimigo
        switch (ultimoLado) {
            case vistoDireita:                 // Procura o inimigo no ultimo lado visto
                moverMotores(velUltimoLado, -velUltimoLado);
                passoQuebrado = 0;            // Reinicia a logica do modo quebrado
                leituraFrente = 0;             // Reinicia a contagem de leituras com sensor F
                primeiraSambada = true;        // Indica que voltou para o primeiro movimento
                break;
            
            default:                           // Caso nao veja na direita, gira para a esquerda
                moverMotores(velUltimoLado, -velUltimoLado);
                passoQuebrado = 0;            // Reinicia a logica do modo quebrado
                leituraFrente = 0;             // Reinicia a contagem de leituras com sensor F
                primeiraSambada = true;        // Indica que voltou para o primeiro movimento
                break;
        }
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
}

#pragma endregion

//========================================//SLOW SEARCH//========================================//

#pragma region SLOW SEARCH

void slowSearch() {
    static int passoSearch = 0;
    static unsigned long tempoPassoSearch = 0;

    modoFurtivo = false; xTaskNotifyGive(swSensorHandle);

    switch(passoSearch) {
        case 0:
            moverMotores(200, 200);
            tempoPassoSearch = millis();
            passoSearch = 1;
            break;

        case 1:
            if (millis() - tempoPassoSearch > 150) passoSearch = 2;
            break;

        case 2:
            moverMotores(0, 0);
            tempoPassoSearch = millis();
            passoSearch = 3;
            break;

        case 3:
            if (millis() - tempoPassoSearch > 75) passoSearch = 4;
            break;

        case 4:
            moverMotores(-200, 200);
            tempoPassoSearch = millis();
            passoSearch = 5;
            break;

        case 5:
            if (millis() - tempoPassoSearch > 75) passoSearch = 6;
            break;

        case 6:
            moverMotores(200, -200);
            tempoPassoSearch = millis();
            passoSearch = 7;
            break;

        case 7:
            if (millis() - tempoPassoSearch > 150) passoSearch = 8;
            break;

        case 8:
            moverMotores(-200, 200);
            tempoPassoSearch = millis();
            passoSearch = 9;
            break;

        case 9:
            if (millis() - tempoPassoSearch > 75) passoSearch = 10;
            break;
                
        case 10:
            moverMotores(0, 0);
            tempoPassoSearch = millis();
            passoSearch = 11;
            break;

        case 11:
            if (millis() - tempoPassoSearch > 150) {
                numIteracoes++;
                passoSearch = 0;
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
}

#pragma endregion

//===============================================================================================//
//=====================================//FINALIZA O ARQUIVO//====================================//
//===============================================================================================//

#endif