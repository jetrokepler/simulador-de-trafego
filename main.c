#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "map.h"
#include "render.h"
#include "clock.h"
#include "traffic_light.h"
#include "vehicle.h"
#include "logger.h"

/* =======================================================================
 * INTEGRANTE 3 - Integração final
 * Arquivo: main.c
 *
 * Cria 24 threads: 1 relógio global + 8 semáforos + 14 carros + 1
 * ambulância, além da própria main que faz o papel de "loop de
 * renderização" (Etapa 3.7 do plano de ação).
 * ===================================================================== */

Map          g_map;                          /* definição real (extern em map.h) */
TrafficLight lights[NUM_INTERSECTIONS];       /* definição real (extern em traffic_light.h) */

static Vehicle vehicles[NUM_VEHICLES];
static pthread_t clock_tid;
static pthread_t light_tids[NUM_INTERSECTIONS];
static pthread_t vehicle_tids[NUM_VEHICLES];

/* Ctrl+C encerra a simulação de forma limpa em vez de matar o processo
 * bruscamente (facilita ver o join de todas as 24 threads). */
static void handle_sigint(int sig) {
    (void)sig;
    simulation_running = 0;

    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);
}

/* Acorda também quem estiver dormindo em um semáforo específico
 * (wait_for_green usa cond_horiz/cond_vert, que são variáveis de
 * condição diferentes da do relógio global), para não deixar nenhuma
 * thread de veículo presa durante o encerramento. */
static void wake_all_traffic_lights(void) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        pthread_mutex_lock(&lights[i].lock);
        pthread_cond_broadcast(&lights[i].cond_horiz);
        pthread_cond_broadcast(&lights[i].cond_vert);
        pthread_mutex_unlock(&lights[i].lock);
    }
}

int main(int argc, char *argv[]) {
    /* Uso opcional: ./simulator [max_ticks]
     * Sem argumento -> roda indefinidamente até Ctrl+C.
     * Com argumento -> roda esse número de ticks e encerra sozinho
     * (útil para demonstrações automatizadas / correção). */
    long max_ticks = -1;
    if (argc > 1) {
        max_ticks = atol(argv[1]);
        if (max_ticks <= 0) {
            fprintf(stderr, "Uso: %s [numero_de_ticks > 0]\n", argv[0]);
            return 1;
        }
    }

    signal(SIGINT, handle_sigint);

    map_init(&g_map);
    lights_init(lights, NUM_INTERSECTIONS);
    clock_init();
    logger_init("simulation.log");

    vehicles_init_all(vehicles, NUM_VEHICLES);

    log_event("Simulacao iniciada: %d veiculos, %d cruzamentos",
              NUM_VEHICLES, NUM_INTERSECTIONS);

    /* --- criação das 24 threads --- */
    pthread_create(&clock_tid, NULL, clock_thread, NULL);

    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_create(&light_tids[i], NULL, traffic_light_thread, &lights[i]);

    for (int i = 0; i < NUM_VEHICLES; i++)
        pthread_create(&vehicle_tids[i], NULL, vehicle_thread, &vehicles[i]);

    /* --- loop de renderização (roda na thread main) --- */
    for (long t = 1; (max_ticks < 0 || t <= max_ticks) && simulation_running; t++) {
        wait_until_tick(t);
        if (!simulation_running) break;
        render_map(&g_map, vehicles, NUM_VEHICLES, lights);
        printf("tick = %ld%s\n", t, max_ticks > 0 ? "" : "  (Ctrl+C para encerrar)");
    }

    /* --- encerramento limpo --- */
    simulation_running = 0;
    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);
    wake_all_traffic_lights();

    for (int i = 0; i < NUM_VEHICLES; i++)
        pthread_join(vehicle_tids[i], NULL);
    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_join(light_tids[i], NULL);
    pthread_join(clock_tid, NULL);

    log_event("Simulacao encerrada");

    vehicles_destroy_all(vehicles, NUM_VEHICLES);
    lights_destroy(lights, NUM_INTERSECTIONS);
    clock_destroy();
    map_destroy(&g_map);
    logger_close();

    printf("Simulacao encerrada com seguranca (todas as 24 threads finalizadas).\n");
    return 0;
}
