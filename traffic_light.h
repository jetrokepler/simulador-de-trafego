#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <pthread.h>
#include <semaphore.h>
#include "map.h"

/* =======================================================================
 * INTEGRANTE 2 - Relógio global, semáforos de trânsito e sincronização
 * Arquivo: traffic_light.h
 *
 * Struct de dados (Etapa 2.2) + protótipos das funções implementadas em
 * traffic_light.c (Etapas 2.3, 2.4, 2.5).
 * ===================================================================== */

typedef enum { GREEN, RED } LightState;

typedef struct {
    int             id;
    LightState      state_horiz;  /* estado para quem vem na horizontal */
    LightState      state_vert;   /* estado para quem vem na vertical   */
    int             green_ticks;
    int             red_ticks;
    long            next_change;
    int             priority;      /* 1 se ambulância pediu prioridade  */
    pthread_mutex_t lock;
    pthread_cond_t  cond_horiz;    /* carros horizontais dormem aqui    */
    pthread_cond_t  cond_vert;     /* carros verticais dormem aqui      */
    sem_t           entry_sem;     /* limita a 1 veículo por vez dentro do cruzamento */
} TrafficLight;

extern TrafficLight lights[NUM_INTERSECTIONS];

/* Inicializa os N semáforos: estado inicial, ticks de verde/vermelho,
 * mutex, variáveis de condição e semáforo de entrada (Etapas 2.2/2.5). */
void lights_init(TrafficLight lights_arr[], int n);

/* Libera mutex/cond/sem de todos os semáforos. */
void lights_destroy(TrafficLight lights_arr[], int n);

/* Thread de um semáforo: alterna verde/vermelho respeitando o relógio
 * global e acorda as threads bloqueadas quando muda de estado
 * (Etapa 2.3). Não faz nada se tl->priority estiver setado (ambulância
 * está usando o cruzamento). */
void *traffic_light_thread(void *arg);

/* Bloqueia (sem consumir CPU) a thread chamadora até que o sinal fique
 * verde para a direção informada (Etapa 2.4). Usada pelos veículos
 * (Integrante 3) antes de entrar em uma célula de cruzamento. */
void wait_for_green(TrafficLight *tl, Direction dir);

#endif /* TRAFFIC_LIGHT_H */
