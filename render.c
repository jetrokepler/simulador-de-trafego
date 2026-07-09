#include <stdio.h>
#include "render.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

static char vehicle_symbol(VehicleType t) {
    switch (t) {
        case CAR_FAST:   return 'R';
        case CAR_MEDIUM: return 'M';
        case CAR_SLOW:   return 'L';
        case AMBULANCE:  return '@';
        default:         return '?';
    }
}

static const char *vehicle_color(VehicleType t) {
    switch (t) {
        case CAR_FAST:   return COLOR_YELLOW;
        case CAR_MEDIUM: return COLOR_BLUE;
        case CAR_SLOW:   return COLOR_CYAN;
        case AMBULANCE:  return COLOR_MAGENTA;
        default:         return COLOR_WHITE;
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
        printf("%s%c %s", vehicle_color(v->type), vehicle_symbol(v->type), COLOR_RESET);
        return;
    }

    if (type == CELL_INTERSECTION) {
        if (lights != NULL) {
            TrafficLight *tl = &lights[inter_id];
            pthread_mutex_lock(&tl->lock);
            LightState state = tl->state_horiz;
            pthread_mutex_unlock(&tl->lock);
            
            if (state == GREEN) {
                printf("%sG %s", COLOR_GREEN, COLOR_RESET);
            } else {
                printf("%sR %s", COLOR_RED, COLOR_RESET);
            }
        } else {
            printf("? ");
        }
        return;
    }

    if (type == CELL_ROAD) {
        printf("%s", road_arrow(dir));
        return;
    }

    printf("\u2588\u2588");
}

void render_map(Map *m, Vehicle *vehicles, int n_vehicles, TrafficLight *lights) {
    printf("\033[H\033[J");

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            render_cell(&m->grid[r][c], vehicles, n_vehicles, lights);
        }
        printf("\n");
    }

    printf("\nLegenda: %s[R]rapido%s  %s[M]medio%s  %s[L]lento%s  %s[@]ambulancia%s  %s[G]verde%s  %s[R]vermelho%s  \u2588\u2588calcada\n",
           COLOR_YELLOW, COLOR_RESET,
           COLOR_BLUE, COLOR_RESET,
           COLOR_CYAN, COLOR_RESET,
           COLOR_MAGENTA, COLOR_RESET,
           COLOR_GREEN, COLOR_RESET,
           COLOR_RED, COLOR_RESET);
}