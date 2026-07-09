#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "map.h"
#include "clock.h"
#include "traffic_light.h"
#include "render.h"

/* =======================================================================
 * demo_test2.c
 *
 * NÃO faz parte da entrega final do Integrante 2 (que é clock.h/clock.c/
 * traffic_light.h/traffic_light.c). Serve para provar que o relógio
 * global e as threads de semáforo funcionam de verdade: cada semáforo
 * alterna sozinho, sem espera ocupada, e o main só acompanha via
 * wait_until_tick + render_map (do Integrante 1).
 *
 * Compilar:
 *   gcc -o demo_test2 demo_test2.c map.c render.c clock.c traffic_light.c -lpthread
 * Executar:
 *   ./demo_test2
 * ===================================================================== */

TrafficLight lights[NUM_INTERSECTIONS]; /* definição real da variável extern */

static Map map;

int main(void) {
    map_init(&map);
    clock_init();
    lights_init(lights, NUM_INTERSECTIONS);

    pthread_t clock_tid;
    pthread_t light_tids[NUM_INTERSECTIONS];

    pthread_create(&clock_tid, NULL, clock_thread, NULL);
    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_create(&light_tids[i], NULL, traffic_light_thread, &lights[i]);

    /* Roda por 40 ticks só para demonstrar as trocas de sinal (com
     * green_ticks = red_ticks = 15, dá pra ver pelo menos uma troca
     * completa em cada cruzamento). */
    for (long t = 1; t <= 40; t++) {
        wait_until_tick(t);
        render_map(&map, NULL, 0, lights);
        printf("tick = %ld\n", t);
    }

    /* Encerramento limpo: acorda todo mundo e junta as threads. */
    simulation_running = 0;
    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);

    pthread_join(clock_tid, NULL);
    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_join(light_tids[i], NULL);

    lights_destroy(lights, NUM_INTERSECTIONS);
    clock_destroy();
    map_destroy(&map);

    printf("Demo do Integrante 2 encerrada sem travar. OK.\n");
    return 0;
}
