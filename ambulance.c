#include "ambulance.h"
#include "traffic_light.h"
#include "logger.h"

void request_priority(int inter_id, Direction dir) {
    TrafficLight *tl = &lights[inter_id];

    pthread_mutex_lock(&tl->lock);
    tl->priority = 1;

    if (dir == DIR_RIGHT || dir == DIR_LEFT) {
        tl->state_horiz = GREEN;
        tl->state_vert  = RED;
        pthread_cond_broadcast(&tl->cond_horiz);
    } else {
        tl->state_vert  = GREEN;
        tl->state_horiz = RED;
        pthread_cond_broadcast(&tl->cond_vert);
    }
    pthread_mutex_unlock(&tl->lock);

    log_event("AMBULANCIA solicitou prioridade no cruzamento %d (direcao %s)",
              inter_id, direction_name(dir));
}

void release_priority(int inter_id) {
    TrafficLight *tl = &lights[inter_id];

    pthread_mutex_lock(&tl->lock);
    tl->priority = 0; 
    pthread_mutex_unlock(&tl->lock);

    log_event("AMBULANCIA liberou o cruzamento %d", inter_id);
}
