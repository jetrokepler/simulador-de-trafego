#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>
#include <stdatomic.h>



typedef struct {
    long            tick;
    pthread_mutex_t lock;
    pthread_cond_t  cond;   
} GlobalClock;

extern GlobalClock g_clock;

extern atomic_int simulation_running;

#define TICK_US 300000

void clock_init(void);

void clock_destroy(void);

void *clock_thread(void *arg);

void wait_until_tick(long target);

#endif
