#include <unistd.h>
#include "clock.h"



GlobalClock g_clock;
atomic_int simulation_running = 1;
long g_tick_us = TICK_US_DEFAULT;

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
        usleep((useconds_t)g_tick_us);

        pthread_mutex_lock(&g_clock.lock);
        g_clock.tick++;
        pthread_cond_broadcast(&g_clock.cond); 
        pthread_mutex_unlock(&g_clock.lock);
    }


    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);

    return NULL;
}

void wait_until_tick(long target) {
    pthread_mutex_lock(&g_clock.lock);
    while (g_clock.tick < target && simulation_running)
        pthread_cond_wait(&g_clock.cond, &g_clock.lock); 
    pthread_mutex_unlock(&g_clock.lock);
}
