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

/* Etapa 4: valor padrão do tick (em microssegundos), usado se nenhum
   argumento de linha de comando for passado. Aumentado de 300ms para 700ms
   para tornar a simulação observável visualmente. Pode ser sobrescrito em
   tempo de execução via g_tick_us (ver main.c). */
#define TICK_US_DEFAULT 700000

extern long g_tick_us;

void clock_init(void);

void clock_destroy(void);

void *clock_thread(void *arg);

void wait_until_tick(long target);

#endif
