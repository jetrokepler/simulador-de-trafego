#include <unistd.h>
#include "clock.h"

/* =======================================================================
 * INTEGRANTE 2 - Relógio global, semáforos de trânsito e sincronização
 * Arquivo: clock.c
 * ===================================================================== */

GlobalClock g_clock;
atomic_int simulation_running = 1;

void clock_init(void) {
    g_clock.tick = 0;
    pthread_mutex_init(&g_clock.lock, NULL);
    pthread_cond_init(&g_clock.cond, NULL);
    simulation_running = 1;
}

void clock_destroy(void) {
    pthread_mutex_destroy(&g_clock.lock);
    pthread_cond_destroy(&g_clock.cond);
}

void *clock_thread(void *arg) {
    (void)arg;

    while (simulation_running) {
        usleep(TICK_US);

        pthread_mutex_lock(&g_clock.lock);
        g_clock.tick++;
        pthread_cond_broadcast(&g_clock.cond); /* acorda todas as threads
                                                    esperando este tick   */
        pthread_mutex_unlock(&g_clock.lock);
    }

    /* Ao encerrar a simulação, acorda geral para que nenhuma thread
     * fique presa para sempre em pthread_cond_wait(). */
    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);

    return NULL;
}

void wait_until_tick(long target) {
    pthread_mutex_lock(&g_clock.lock);
    while (g_clock.tick < target && simulation_running)
        pthread_cond_wait(&g_clock.cond, &g_clock.lock); /* dorme, zero
                                                              consumo de CPU */
    pthread_mutex_unlock(&g_clock.lock);
}
