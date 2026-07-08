#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>
#include <stdatomic.h>

/* =======================================================================
 * INTEGRANTE 2 - Relógio global, semáforos de trânsito e sincronização
 * Arquivo: clock.h
 *
 * O relógio global é a ÚNICA fonte de avanço de tempo da simulação.
 * Nenhuma thread (carro, ambulância, semáforo) verifica o tempo em loop
 * ocupado: todas dormem em pthread_cond_wait() até serem acordadas pelo
 * broadcast do relógio.
 * ===================================================================== */

typedef struct {
    long            tick;
    pthread_mutex_t lock;
    pthread_cond_t  cond;   /* broadcast a cada tick */
} GlobalClock;

extern GlobalClock g_clock;

/* atomic_int (C11) em vez de "volatile int": volatile evita apenas
 * otimizações do compilador, mas não é uma garantia de sincronização
 * entre threads (ThreadSanitizer acusa a leitura/escrita concorrente
 * como data race). atomic_int usa operações atômicas de verdade e pode
 * ser lido/escrito normalmente (if (simulation_running), etc.) sem
 * qualquer outra mudança no resto do código. */
extern atomic_int simulation_running;

/* Duração de cada tick em microssegundos (ex.: 300ms). */
#define TICK_US 300000

/* Inicializa g_clock (tick = 0) e simulation_running = 1. */
void clock_init(void);

/* Libera mutex/cond do relógio global. */
void clock_destroy(void);

/* Thread do relógio: dorme TICK_US, incrementa g_clock.tick e acorda
 * (broadcast) todas as threads que estão esperando um tick (Etapa 2.1). */
void *clock_thread(void *arg);

/* Função auxiliar usada por veículos e semáforos: bloqueia a thread
 * chamadora (sem consumir CPU) até que g_clock.tick >= target, ou até a
 * simulação ser encerrada (Etapa 2.1). Esta é a função que garante que
 * "os carros não devem usar loops infinitos testando o tempo". */
void wait_until_tick(long target);

#endif /* CLOCK_H */
