#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "render.h"

/* =======================================================================
 * demo_test.c
 *
 * NÃO faz parte da entrega final do Integrante 1 (que é só map.h/map.c/
 * render.h/render.c). Serve apenas para provar, de forma isolada, que o
 * mapa e a renderização funcionam antes de serem integrados por
 * Integrantes 2 e 3 (threads reais de semáforo/veículo/relógio).
 *
 * Compilar:
 *   gcc -o demo_test demo_test.c map.c render.c -lpthread
 * Executar:
 *   ./demo_test
 * ===================================================================== */

static Map map;
TrafficLight lights[NUM_INTERSECTIONS]; /* definição real da variável extern declarada em traffic_light.h */
static Vehicle vehicles[4];

static void init_fake_lights(void) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        lights[i].id = i;
        /* alterna verde/vermelho só para demonstrar visualmente */
        lights[i].state_horiz = (i % 2 == 0) ? GREEN : RED;
        lights[i].state_vert  = (i % 2 == 0) ? RED   : GREEN;
        pthread_mutex_init(&lights[i].lock, NULL);
        pthread_cond_init(&lights[i].cond_horiz, NULL);
        pthread_cond_init(&lights[i].cond_vert, NULL);
        sem_init(&lights[i].entry_sem, 0, 1);
    }
}

static void place_vehicle(int idx, int id, VehicleType type, int row, int col) {
    vehicles[idx].id       = id;
    vehicles[idx].type     = type;
    vehicles[idx].row      = row;
    vehicles[idx].col      = col;
    vehicles[idx].active   = 1;
    vehicles[idx].speed    = 1;
    vehicles[idx].route    = NULL;
    vehicles[idx].route_len = 0;
    vehicles[idx].route_idx = 0;

    map.grid[row][col].occupied   = 1;
    map.grid[row][col].vehicle_id = id;
}

int main(void) {
    map_init(&map);
    init_fake_lights();

    /* Alguns veículos de exemplo em posições válidas de pista. */
    place_vehicle(0, 0, CAR_FAST,   3, 5);   /* mão dupla superior, faixa de ida */
    place_vehicle(1, 1, CAR_MEDIUM, 4, 45);  /* mão dupla superior, faixa de volta */
    place_vehicle(2, 2, CAR_SLOW,   8, 25);  /* mão única 1 */
    place_vehicle(3, 3, AMBULANCE,  16, 30); /* mão dupla inferior */

    render_map(&map, vehicles, 4, lights);

    map_destroy(&map);
    return 0;
}
