#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "vehicle.h"
#include "clock.h"
#include "traffic_light.h"
#include "ambulance.h"
#include "logger.h"

/* =======================================================================
 * INTEGRANTE 3 - Veículos, ambulância, antideadlock, integração
 * Arquivo: vehicle.c
 *
 * ROTAS (Etapa 3.2): pré-definidas, circulares, retangulares. Cada rota
 * usa duas ruas horizontais paralelas (uma para "ida", outra para
 * "volta") e as duas vias verticais como conectoras. Isso garante que
 * nenhum veículo sai do mapa ou anda contra a mão da via, pois cada
 * trecho da rota é construído seguindo exatamente a direção permitida
 * daquela pista (map.c / allowed_dir).
 *
 * 4 rotas são definidas:
 *   R1 - laço externo:  linha 3 (->, mão dupla sup.) / col 39 (v) /
 *                        linha 17 (<-, mão dupla inf.) / col 15 (^)
 *   R2 - usa mão única 1 (linha 8) como trecho de ida
 *   R3 - usa mão única 2 (linha 12) como trecho de ida
 *   R4 - laço interno:  linha 4 (<-, mão dupla sup.) / col 16 (v) /
 *                        linha 16 (->, mão dupla inf.) / col 38 (^)
 * ===================================================================== */

static Direction direction_between(int r1, int c1, int r2, int c2) {
    if (r1 == r2 && c2 == c1 + 1) return DIR_RIGHT;
    if (r1 == r2 && c2 == c1 - 1) return DIR_LEFT;
    if (c1 == c2 && r2 == r1 - 1) return DIR_UP;
    if (c1 == c2 && r2 == r1 + 1) return DIR_DOWN;
    return DIR_NONE;
}

/* Adiciona ao buffer todas as células entre (r1,c1) [exclusive] e
 * (r2,c2) [inclusive], andando em linha reta (mesma linha ou mesma
 * coluna). Retorna o novo comprimento usado do buffer. */
static int append_leg(int *buf, int len, int r1, int c1, int r2, int c2) {
    if (r1 == r2) {
        int step = (c2 > c1) ? 1 : -1;
        for (int c = c1 + step; ; c += step) {
            buf[len++] = r1 * MAP_COLS + c;
            if (c == c2) break;
        }
    } else {
        int step = (r2 > r1) ? 1 : -1;
        for (int r = r1 + step; ; r += step) {
            buf[len++] = r * MAP_COLS + c1;
            if (r == r2) break;
        }
    }
    return len;
}

/* Constrói uma rota circular retangular fechada:
 *   (r1,c1) -> (r1,c2) -> (r2,c2) -> (r2,c1) -> ~(r1,c1)
 * O último trecho pára UMA célula antes do ponto inicial; o próprio
 * wrap-around de route_idx (route_idx + 1) % route_len fecha o laço com
 * um passo válido e adjacente, sem duplicar a célula inicial. */
static int *build_rect_route(int r1, int c1, int r2, int c2, int *out_len) {
    int cap = 2 * (abs(r2 - r1) + abs(c2 - c1)) + 4;
    int *buf = malloc(sizeof(int) * (size_t)cap);
    int len = 0;

    buf[len++] = r1 * MAP_COLS + c1;                 /* ponto inicial       */
    len = append_leg(buf, len, r1, c1, r1, c2);       /* topo                */
    len = append_leg(buf, len, r1, c2, r2, c2);       /* lateral (conector)  */
    len = append_leg(buf, len, r2, c2, r2, c1);       /* base                */

    /* fecha o laço subindo/descendo até uma célula antes do início */
    int step = (r1 > r2) ? 1 : -1;
    for (int r = r2 + step; r != r1; r += step)
        buf[len++] = r * MAP_COLS + c1;

    *out_len = len;
    return buf;
}

int is_valid_move(int nr, int nc, Direction dir) {
    if (!map_in_bounds(nr, nc)) return 0;

    const Cell *next = &g_map.grid[nr][nc];
    if (next->type == CELL_WALL) return 0;

    /* Em pista comum, a direção do veículo precisa bater com a direção
     * da via (regra 3.2/4.2 do enunciado). Em cruzamentos, quem decide
     * se pode entrar é o semáforo (wait_for_green / prioridade), então
     * não recusamos aqui. */
    if (next->type == CELL_ROAD && next->allowed_dir != dir) return 0;

    return dir != DIR_NONE;
}

void try_move(Vehicle *v) {
    int target_idx = v->route[v->route_idx];
    int nr = target_idx / MAP_COLS;
    int nc = target_idx % MAP_COLS;

    Direction dir = direction_between(v->row, v->col, nr, nc);
    if (!is_valid_move(nr, nc, dir)) return; /* segurança extra; não deveria ocorrer com rotas válidas */

    v->dir = dir;

    int cur_idx  = v->row * MAP_COLS + v->col;
    int next_idx = nr * MAP_COLS + nc;

    Cell *cur_cell  = &g_map.grid[v->row][v->col];
    Cell *next_cell = &g_map.grid[nr][nc];
    int   inter_id  = next_cell->intersection_id;

    /* 1) Checagem de semáforo antes de tentar qualquer lock de célula. */
    if (inter_id >= 0) {
        if (v->type == AMBULANCE) {
            request_priority(inter_id, dir); /* força verde, sem pular exclusão mútua */
        } else {
            wait_for_green(&lights[inter_id], dir); /* dorme sem consumir CPU */
        }
    }

    /* 2) Antideadlock: ordenação global de locks por índice linear.
     * Sempre adquire primeiro o lock de menor índice, e nunca fica
     * segurando um lock enquanto espera outro (trylock + desistência). */
    Cell *first  = (cur_idx < next_idx) ? cur_cell : next_cell;
    Cell *second = (cur_idx < next_idx) ? next_cell : cur_cell;

    pthread_mutex_lock(&first->lock);
    if (pthread_mutex_trylock(&second->lock) != 0) {
        pthread_mutex_unlock(&first->lock);
        if (v->type == AMBULANCE && inter_id >= 0) release_priority(inter_id);
        return; /* célula ocupada: não segura nada, tenta de novo no próximo tick */
    }

    /* 3) Semáforo de entrada do cruzamento: só 1 veículo por vez dentro. */
    if (inter_id >= 0) sem_wait(&lights[inter_id].entry_sem);

    /* Move de fato. */
    cur_cell->occupied    = 0;
    cur_cell->vehicle_id  = -1;
    v->row = nr;
    v->col = nc;
    next_cell->occupied   = 1;
    next_cell->vehicle_id = v->id;

    if (inter_id >= 0) sem_post(&lights[inter_id].entry_sem);

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);

    if (v->type == AMBULANCE && inter_id >= 0) release_priority(inter_id);

    v->route_idx = (v->route_idx + 1) % v->route_len;
}

void *vehicle_thread(void *arg) {
    Vehicle *v = (Vehicle *)arg;
    long next_tick = 1;

    log_event("Thread do veiculo %d (tipo %d) iniciada em (%d,%d)",
              v->id, v->type, v->row, v->col);

    while (v->active && simulation_running) {
        wait_until_tick(next_tick);
        if (!simulation_running) break;
        try_move(v);
        next_tick += v->speed;
    }

    log_event("Thread do veiculo %d encerrada", v->id);
    return NULL;
}

/* ---- Montagem das rotas e inicialização dos veículos (Etapas 3.1/3.2) --- */

#define NUM_ROUTE_TEMPLATES 4

static void place_on_map(Vehicle *v) {
    g_map.grid[v->row][v->col].occupied   = 1;
    g_map.grid[v->row][v->col].vehicle_id = v->id;
}

void vehicles_init_all(Vehicle vehicles[], int n) {
    int *routes[NUM_ROUTE_TEMPLATES];
    int  route_lens[NUM_ROUTE_TEMPLATES];

    /* R1: laço externo (mão dupla superior/inferior + verticais externas) */
    routes[0] = build_rect_route(3, 15, 17, 39, &route_lens[0]);
    /* R2: mão única 1 (linha 8) + verticais externas */
    routes[1] = build_rect_route(8, 15, 17, 39, &route_lens[1]);
    /* R3: mão única 2 (linha 12) + verticais externas */
    routes[2] = build_rect_route(12, 15, 17, 39, &route_lens[2]);
    /* R4: laço interno (mão dupla superior/inferior + verticais internas) */
    routes[3] = build_rect_route(4, 38, 16, 16, &route_lens[3]);

    int n_cars = n - 1; /* último índice é sempre a ambulância */

    for (int i = 0; i < n_cars; i++) {
        Vehicle *v = &vehicles[i];
        int r = i % NUM_ROUTE_TEMPLATES;

        v->id     = i;
        v->type   = (VehicleType)(i % 3); /* alterna CAR_FAST/MEDIUM/SLOW */
        v->speed  = (v->type == CAR_FAST) ? 1 : (v->type == CAR_MEDIUM) ? 2 : 4;
        v->route  = routes[r];
        v->route_len = route_lens[r];
        /* espalha os carros ao longo da própria rota, evitando que todos
         * comecem empilhados na mesma célula */
        v->route_idx = (i * 5) % v->route_len;
        v->active = 1;

        int start_idx = v->route[(v->route_idx == 0) ? (v->route_len - 1)
                                                       : (v->route_idx - 1)];
        v->row = start_idx / MAP_COLS;
        v->col = start_idx % MAP_COLS;
        v->dir = DIR_NONE;

        place_on_map(v);
    }

    /* Ambulância: usa o laço externo (R1), velocidade máxima. */
    Vehicle *amb = &vehicles[n - 1];
    amb->id        = n - 1;
    amb->type      = AMBULANCE;
    amb->speed     = 1;
    amb->route     = routes[0];
    amb->route_len = route_lens[0];
    amb->route_idx = 0;
    amb->active    = 1;

    int start_idx = amb->route[amb->route_len - 1];
    amb->row = start_idx / MAP_COLS;
    amb->col = start_idx % MAP_COLS;
    amb->dir = DIR_NONE;
    place_on_map(amb);
}

void vehicles_destroy_all(Vehicle vehicles[], int n) {
    /* As NUM_ROUTE_TEMPLATES rotas são compartilhadas entre vários
     * veículos; para não liberar o mesmo ponteiro duas vezes, liberamos
     * apenas o conjunto de ponteiros únicos. */
    int freed[NUM_ROUTE_TEMPLATES] = {0};
    int *unique_ptrs[NUM_ROUTE_TEMPLATES];
    int n_unique = 0;

    for (int i = 0; i < n; i++) {
        int already = 0;
        for (int j = 0; j < n_unique; j++)
            if (unique_ptrs[j] == vehicles[i].route) { already = 1; break; }
        if (!already && n_unique < NUM_ROUTE_TEMPLATES) {
            unique_ptrs[n_unique++] = vehicles[i].route;
        }
    }
    for (int j = 0; j < n_unique; j++) {
        free(unique_ptrs[j]);
        freed[j] = 1;
    }
    (void)freed;
}
