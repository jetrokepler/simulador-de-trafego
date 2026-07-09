#include <stdio.h>
#include "traffic_light.h"
#include "clock.h"

#define DEFAULT_GREEN_TICKS 15
#define DEFAULT_RED_TICKS   15

void lights_init(TrafficLight lights_arr[], int n) {
    for (int i = 0; i < n; i++) {
        TrafficLight *tl = &lights_arr[i];

        tl->id          = i;
        tl->green_ticks = DEFAULT_GREEN_TICKS;
        tl->red_ticks   = DEFAULT_RED_TICKS;
        tl->priority    = 0;
        tl->state_horiz = GREEN;
        tl->state_vert  = RED;
        tl->next_change = tl->green_ticks;

        pthread_mutex_init(&tl->lock, NULL);
        pthread_cond_init(&tl->cond_horiz, NULL);
        pthread_cond_init(&tl->cond_vert, NULL);
        sem_init(&tl->entry_sem, 0, 1);
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
        long target = tl->next_change; 
        pthread_mutex_unlock(&tl->lock);

        wait_until_tick(target);                         
        if (!simulation_running) break;

        pthread_mutex_lock(&tl->lock);

        if (!tl->priority) {

            if (tl->state_horiz == GREEN) {
                tl->state_horiz = RED;
                tl->state_vert  = GREEN;
                tl->next_change = g_clock.tick + tl->red_ticks;
            } else {
                tl->state_horiz = GREEN;
                tl->state_vert  = RED;
                tl->next_change = g_clock.tick + tl->green_ticks;
            }


            pthread_cond_broadcast(&tl->cond_horiz);
            pthread_cond_broadcast(&tl->cond_vert);
        } else {

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
        pthread_cond_wait(cond, &tl->lock);
    pthread_mutex_unlock(&tl->lock);
}
