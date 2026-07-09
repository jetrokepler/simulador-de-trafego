#include <stdio.h>
#include "render.h"

static char vehicle_symbol(VehicleType t) {
    switch (t) {
        case CAR_FAST:   return 'R';
        case CAR_MEDIUM: return 'M';
        case CAR_SLOW:   return 'L';
        case AMBULANCE:  return '@';
        default:         return '?';
    }
}


static const Vehicle *find_vehicle(const Vehicle *vehicles, int n_vehicles, int vehicle_id) {
    if (vehicle_id < 0) return NULL;
    for (int i = 0; i < n_vehicles; i++) {
        if (vehicles[i].active && vehicles[i].id == vehicle_id)
            return &vehicles[i];
    }
    return NULL;
}

static const char *road_arrow(Direction d) {
    switch (d) {
        case DIR_RIGHT: return "\u2192 ";
        case DIR_LEFT:  return "\u2190 ";
        case DIR_UP:    return "\u2191 ";
        case DIR_DOWN:  return "\u2193 ";
        default:        return "  ";
    }
}

static void render_cell(Cell *cell, const Vehicle *vehicles, int n_vehicles,
                         TrafficLight *lights) {

    pthread_mutex_lock(&cell->lock);
    int       occupied   = cell->occupied;
    int       vehicle_id = cell->vehicle_id;
    CellType  type       = cell->type;
    Direction dir        = cell->allowed_dir;
    int       inter_id   = cell->intersection_id;
    pthread_mutex_unlock(&cell->lock);

    const Vehicle *v = find_vehicle(vehicles, n_vehicles, vehicle_id);
    if (occupied && v != NULL) {
        printf("%c ", vehicle_symbol(v->type));
        return;
    }

    if (type == CELL_INTERSECTION) {
        if (lights != NULL) {
            TrafficLight *tl = &lights[inter_id];
            pthread_mutex_lock(&tl->lock);
            LightState state = tl->state_horiz;
            pthread_mutex_unlock(&tl->lock);
            printf("%s ", state == GREEN ? "G" : "R");
        } else {
            printf("? ");
        }
        return;
    }


    if (type == CELL_ROAD) {
        printf("%s", road_arrow(dir));
        return;
    }


    printf("  "); /* ██ */
}

void render_map(Map *m, Vehicle *vehicles, int n_vehicles, TrafficLight *lights) {

    printf("\033[H\033[J");

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            render_cell(&m->grid[r][c], vehicles, n_vehicles, lights);
        }
        printf("\n");
    }

    printf("\nLegenda: [R]=rapido  [M]=medio  [L]=lento  [@]=ambulancia  "
           "[G]=verde  [R]=vermelho  \u2588\u2588=calcada\n");
}
