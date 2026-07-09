#ifndef RENDER_H
#define RENDER_H

#include "map.h"
#include "vehicle.h"
#include "traffic_light.h"

void render_map(Map *m, Vehicle *vehicles, int n_vehicles, TrafficLight *lights);

#endif
