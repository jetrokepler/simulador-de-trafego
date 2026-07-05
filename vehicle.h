#ifndef VEHICLE_H
#define VEHICLE_H

#include "map.h"

/* =======================================================================
 * INTEGRANTE 3 - Veículos, ambulância, antideadlock, integração
 * Arquivo: vehicle.h
 *
 * Struct de dados (Etapa 3.1, mesma definição já usada por render.c do
 * Integrante 1) + protótipos das funções implementadas em vehicle.c.
 * ===================================================================== */

typedef enum {
    CAR_FAST,    /* R - move a cada 1 tick               */
    CAR_MEDIUM,  /* M - move a cada 2 ticks               */
    CAR_SLOW,    /* L - move a cada 4 ticks               */
    AMBULANCE    /* @ - move a cada 1 tick + prioridade   */
} VehicleType;

typedef struct {
    int         id;
    VehicleType type;
    int         row, col;   /* posição atual no grid            */
    Direction   dir;        /* direção atual (recalculada a cada passo da rota) */
    int         speed;      /* 1, 2 ou 4 ticks por movimento     */
    int        *route;      /* array de índices lineares (row*COLS+col), rota circular */
    int         route_len;
    int         route_idx;  /* posição atual na rota             */
    int         active;     /* 0 = thread encerrada, 1 = ativa   */
} Vehicle;

/* Entre 10 e 20 veículos simultâneos: 14 carros + 1 ambulância. */
#define NUM_VEHICLES 15

/* Cria as rotas circulares pré-definidas (Etapa 3.2) e inicializa todos
 * os NUM_VEHICLES veículos, posicionando-os no mapa (g_map). O último
 * veículo (índice n-1) é sempre a ambulância. */
void vehicles_init_all(Vehicle vehicles[], int n);

/* Libera a memória das rotas alocadas por vehicles_init_all(). */
void vehicles_destroy_all(Vehicle vehicles[], int n);

/* Thread de cada veículo (Etapa 3.3): dorme via wait_until_tick() e
 * tenta avançar uma célula na rota a cada `speed` ticks. */
void *vehicle_thread(void *arg);

/* Tenta mover o veículo uma célula à frente na rota (Etapa 3.4).
 * Aplica, nesta ordem: 1) checagem de semáforo (wait_for_green, ou
 * request_priority/release_priority se for ambulância); 2) ordenação
 * global de locks por índice linear (antideadlock); 3) sem_t de entrada
 * do cruzamento (no máx. 1 veículo por vez dentro dele). */
void try_move(Vehicle *v);

/* Verifica se (nr, nc) é um destino válido para o veículo: dentro do
 * mapa, não é parede, e -- se for célula comum de pista -- respeita a
 * direção da via. Células de cruzamento são sempre aceitas aqui porque
 * a permissão real de entrada é decidida pelo semáforo. */
int is_valid_move(int nr, int nc, Direction dir);

#endif /* VEHICLE_H */
