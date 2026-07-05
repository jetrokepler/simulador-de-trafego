#ifndef RENDER_H
#define RENDER_H

#include "map.h"
#include "vehicle.h"
#include "traffic_light.h"

/* Desenha o estado atual da simulação em ASCII no terminal.
 * Deve ser chamada a cada tick pela thread principal (main.c,
 * Integrante 3), depois que o relógio global avança. */
void render_map(Map *m, Vehicle *vehicles, int n_vehicles, TrafficLight *lights);

#endif /* RENDER_H */
