#include <stdio.h>
#include "traffic_light.h"
#include "clock.h"

/* =======================================================================
 * INTEGRANTE 2 - Relógio global, semáforos de trânsito e sincronização
 * Arquivo: traffic_light.c
 *
 * Cada um dos NUM_INTERSECTIONS cruzamentos tem sua PRÓPRIA thread de
 * semáforo (Etapa 2.3), que alterna verde/vermelho respeitando o relógio
 * global (nunca faz sleep/poll: dorme via wait_until_tick). Os carros
 * (Integrante 3) chamam wait_for_green() e dormem em uma variável de
 * condição até o sinal abrir (Etapa 2.4). Um sem_t por cruzamento
 * garante que só um veículo esteja fisicamente dentro do "+----+" por
 * vez, mesmo com sinal verde para as duas direções simultaneamente
 * (Etapa 2.5).
 * ===================================================================== */

/* Duração padrão dos ciclos de sinal, em ticks. Poderiam variar por
 * cruzamento; aqui usamos o mesmo valor para todos por simplicidade. */
#define DEFAULT_GREEN_TICKS 15
#define DEFAULT_RED_TICKS   15

void lights_init(TrafficLight lights_arr[], int n) {
    for (int i = 0; i < n; i++) {
        TrafficLight *tl = &lights_arr[i];

        tl->id          = i;
        tl->green_ticks = DEFAULT_GREEN_TICKS;
        tl->red_ticks   = DEFAULT_RED_TICKS;
        tl->priority    = 0;

        /* Todos começam com a horizontal verde e a vertical vermelha;
         * a primeira troca acontece em tick = green_ticks. */
        tl->state_horiz = GREEN;
        tl->state_vert  = RED;
        tl->next_change = tl->green_ticks;

        pthread_mutex_init(&tl->lock, NULL);
        pthread_cond_init(&tl->cond_horiz, NULL);
        pthread_cond_init(&tl->cond_vert, NULL);
        sem_init(&tl->entry_sem, 0, 1); /* 1 veículo por vez no cruzamento */
    }
}

void lights_destroy(TrafficLight lights_arr[], int n) {
    for (int i = 0; i < n; i++) {
        TrafficLight *tl = &lights_arr[i];
        pthread_mutex_destroy(&tl->lock);
        pthread_cond_destroy(&tl->cond_horiz);
        pthread_cond_destroy(&tl->cond_vert);
        sem_destroy(&tl->entry_sem);
    }
}

void *traffic_light_thread(void *arg) {
    TrafficLight *tl = (TrafficLight *)arg;

    while (simulation_running) {
        pthread_mutex_lock(&tl->lock);
        long target = tl->next_change; /* lido sob o mutex: outras threads
                                           (esta mesma, mais adiante, e
                                           potencialmente a ambulância)
                                           escrevem next_change sob o
                                           mesmo lock                    */
        pthread_mutex_unlock(&tl->lock);

        wait_until_tick(target); /* dorme até a hora de trocar, sem
                                     consumir CPU                        */
        if (!simulation_running) break;

        pthread_mutex_lock(&tl->lock);

        if (!tl->priority) {
            /* Só alterna o ciclo normal se a ambulância não estiver
             * segurando prioridade neste cruzamento (Etapa 4/6). */
            if (tl->state_horiz == GREEN) {
                tl->state_horiz = RED;
                tl->state_vert  = GREEN;
                tl->next_change = g_clock.tick + tl->red_ticks;
            } else {
                tl->state_horiz = GREEN;
                tl->state_vert  = RED;
                tl->next_change = g_clock.tick + tl->green_ticks;
            }

            /* Acorda todos os carros bloqueados neste semáforo; quem
             * ainda estiver no vermelho volta a dormir sozinho ao
             * reavaliar a condição do while em wait_for_green(). */
            pthread_cond_broadcast(&tl->cond_horiz);
            pthread_cond_broadcast(&tl->cond_vert);
        } else {
            /* Em prioridade: não muda de estado agora, só reagenda a
             * checagem para o próximo tick até a ambulância liberar. */
            tl->next_change = g_clock.tick + 1;
        }

        pthread_mutex_unlock(&tl->lock);
    }

    return NULL;
}

void wait_for_green(TrafficLight *tl, Direction dir) {
    pthread_cond_t *cond  = (dir == DIR_RIGHT || dir == DIR_LEFT)
                                ? &tl->cond_horiz : &tl->cond_vert;
    LightState *state     = (dir == DIR_RIGHT || dir == DIR_LEFT)
                                ? &tl->state_horiz : &tl->state_vert;

    pthread_mutex_lock(&tl->lock);
    while (*state == RED && simulation_running)
        pthread_cond_wait(cond, &tl->lock); /* dorme sem consumir CPU ✓ */
    pthread_mutex_unlock(&tl->lock);
}
