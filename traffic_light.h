#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <pthread.h>
#include <semaphore.h>
#include "map.h"

typedef enum { GREEN, RED } LightState;

typedef struct {
    int             id;
    LightState      state_horiz;  
    LightState      state_vert;  
    int             green_ticks;
    int             red_ticks;
    long            next_change;
    int             priority;    
    pthread_mutex_t lock;
    pthread_cond_t  cond_horiz;    
    pthread_cond_t  cond_vert;    
    sem_t           entry_sem;   
} TrafficLight;

extern TrafficLight lights[NUM_INTERSECTIONS];
void lights_init(TrafficLight lights_arr[], int n);

void lights_destroy(TrafficLight lights_arr[], int n);

void *traffic_light_thread(void *arg);


void wait_for_green(TrafficLight *tl, Direction dir);

#endif
