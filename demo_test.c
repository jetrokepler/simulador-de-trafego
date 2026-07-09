#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "render.h"

static Map map;
TrafficLight lights[NUM_INTERSECTIONS]; 
static Vehicle vehicles[4];

static void init_fake_lights(void) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        lights[i].id = i;

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


    place_vehicle(0, 0, CAR_FAST,   3, 5); 
    place_vehicle(1, 1, CAR_MEDIUM, 4, 45);  
    place_vehicle(2, 2, CAR_SLOW,   8, 25); 
    place_vehicle(3, 3, AMBULANCE,  16, 30); 

    render_map(&map, vehicles, 4, lights);

    map_destroy(&map);
    return 0;
}
