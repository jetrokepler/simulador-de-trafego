#include "ambulance.h"
#include "traffic_light.h"
#include "logger.h"

/* =======================================================================
 * INTEGRANTE 3 - Ambulância
 * Arquivo: ambulance.c
 *
 * A ambulância roda como uma thread de veículo normal (vehicle_thread),
 * mas antes de entrar em um cruzamento chama request_priority() em vez
 * de wait_for_green(). Isso força o sinal a abrir na direção dela sem
 * pular a exclusão mútua de célula: ela ainda precisa adquirir o mutex
 * da célula normalmente em try_move() (regra 4.1/6 do enunciado).
 * ===================================================================== */

void request_priority(int inter_id, Direction dir) {
    TrafficLight *tl = &lights[inter_id];

    pthread_mutex_lock(&tl->lock);
    tl->priority = 1; /* impede a thread do semáforo de alternar sozinha
                          enquanto a ambulância estiver usando o cruzamento */

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
    tl->priority = 0; /* devolve o controle ao ciclo normal da thread de semáforo */
    pthread_mutex_unlock(&tl->lock);

    log_event("AMBULANCIA liberou o cruzamento %d", inter_id);
}
