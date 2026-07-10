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



Map          g_map;                        
TrafficLight lights[NUM_INTERSECTIONS];       

static Vehicle vehicles[NUM_VEHICLES];
static pthread_t clock_tid;
static pthread_t light_tids[NUM_INTERSECTIONS];
static pthread_t vehicle_tids[NUM_VEHICLES];

static void handle_sigint(int sig) {
    (void)sig;
    simulation_running = 0;

    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);
}


/* Verificacao de sanidade (suporte ao item de checklist "Exclusao mutua" do
   plano de acao): a cada tick, confirma que nenhum par de veiculos ativos
   ocupa a mesma celula. E O(n^2) com n=NUM_VEHICLES (15), custo desprezivel.
   Serve como rede de seguranca automatizada para detectar regressao do bug
   original (ausencia de checagem de ocupacao em try_move). */
static void assert_no_overlap(Vehicle vehicles[], int n, long tick) {
    for (int i = 0; i < n; i++) {
        if (!vehicles[i].active) continue;
        for (int j = i + 1; j < n; j++) {
            if (!vehicles[j].active) continue;
            if (vehicles[i].row == vehicles[j].row &&
                vehicles[i].col == vehicles[j].col) {
                log_event("ERRO CRITICO [tick %ld]: veiculos %d e %d SOBREPOSTOS em (%d,%d)",
                          tick, vehicles[i].id, vehicles[j].id,
                          vehicles[i].row, vehicles[i].col);
            }
        }
    }
}

static void wake_all_traffic_lights(void) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        pthread_mutex_lock(&lights[i].lock);
        pthread_cond_broadcast(&lights[i].cond_horiz);
        pthread_cond_broadcast(&lights[i].cond_vert);
        pthread_mutex_unlock(&lights[i].lock);
    }
}

int main(int argc, char *argv[]) {
    long max_ticks = -1;
    if (argc > 1) {
        max_ticks = atol(argv[1]);
        if (max_ticks <= 0) {
            fprintf(stderr, "Uso: %s [numero_de_ticks > 0] [tick_us > 0]\n", argv[0]);
            return 1;
        }
    }

    /* Etapa 4: segundo argumento opcional controla a duracao do tick em
       microssegundos (padrao: TICK_US_DEFAULT, definido em clock.h).
       Valores menores = simulacao mais rapida; valores maiores = mais facil
       de observar visualmente. Exemplo: ./simulador 200 800000 */
    if (argc > 2) {
        long tick_us = atol(argv[2]);
        if (tick_us <= 0) {
            fprintf(stderr, "Uso: %s [numero_de_ticks > 0] [tick_us > 0]\n", argv[0]);
            return 1;
        }
        g_tick_us = tick_us;
    }

    signal(SIGINT, handle_sigint);

    map_init(&g_map);
    lights_init(lights, NUM_INTERSECTIONS);
    clock_init();
    logger_init("simulation.log");

    vehicles_init_all(vehicles, NUM_VEHICLES);

    log_event("Simulacao iniciada: %d veiculos, %d cruzamentos",
              NUM_VEHICLES, NUM_INTERSECTIONS);

   
    pthread_create(&clock_tid, NULL, clock_thread, NULL);

    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_create(&light_tids[i], NULL, traffic_light_thread, &lights[i]);

    for (int i = 0; i < NUM_VEHICLES; i++)
        pthread_create(&vehicle_tids[i], NULL, vehicle_thread, &vehicles[i]);

   
    for (long t = 1; (max_ticks < 0 || t <= max_ticks) && simulation_running; t++) {
        wait_until_tick(t);
        if (!simulation_running) break;
        render_map(&g_map, vehicles, NUM_VEHICLES, lights);
        assert_no_overlap(vehicles, NUM_VEHICLES, t);
        printf("tick = %ld%s\n", t, max_ticks > 0 ? "" : "  (Ctrl+C para encerrar)");
    }

   
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
