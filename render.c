#include <stdio.h>
#include "render.h"

/* =======================================================================
 * INTEGRANTE 1 - Mapa, estruturas de dados e visualização ASCII
 * Arquivo: render.c
 *
 * Estratégia de renderização (cada célula ocupa 2 colunas de terminal,
 * para manter o grid alinhado):
 *
 *   1. Se há veículo na célula      -> símbolo do veículo (R/M/L/@)
 *   2. Senão, se é cruzamento       -> "G " (verde) ou "R " (vermelho)
 *   3. Senão, se é pista com dir.   -> seta (->, <-, ^, v)
 *   4. Senão (parede/calçada)       -> "██"
 *
 * OBS: como no layout definido em map.c todo cruzamento nasce do
 * encontro de uma banda horizontal com uma via vertical, a célula de
 * cruzamento sempre guarda em allowed_dir a direção da faixa horizontal
 * que a atravessa; por isso o estado exibido é o do semáforo horizontal
 * (lights[id].state_horiz). Isso é uma simplificação apenas de exibição:
 * o controle real de quem pode entrar (horizontal x vertical) é feito
 * pelos Integrantes 2 e 3 usando state_horiz / state_vert / entry_sem.
 * ===================================================================== */

static char vehicle_symbol(VehicleType t) {
    switch (t) {
        case CAR_FAST:   return 'R';
        case CAR_MEDIUM: return 'M';
        case CAR_SLOW:   return 'L';
        case AMBULANCE:  return '@';
        default:         return '?';
    }
}

/* Procura, entre os veículos ativos, aquele cujo id bate com vehicle_id.
 * Retorna NULL se não encontrar (célula livre ou dado inconsistente). */
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
        case DIR_RIGHT: return "\u2192 "; /* -> */
        case DIR_LEFT:  return "\u2190 "; /* <- */
        case DIR_UP:    return "\u2191 "; /* ^  */
        case DIR_DOWN:  return "\u2193 "; /* v  */
        default:        return "  ";
    }
}

static void render_cell(Cell *cell, const Vehicle *vehicles, int n_vehicles,
                         TrafficLight *lights) {
    /* Lê os campos mutáveis da célula sob o mutex dela, para não
     * disputar leitura/escrita com as threads de veículo (que alteram
     * occupied/vehicle_id dentro de try_move). type/allowed_dir/
     * intersection_id nunca mudam após map_init(), mas lemos tudo junto
     * por simplicidade e porque o lock é breve. */
    pthread_mutex_lock(&cell->lock);
    int       occupied   = cell->occupied;
    int       vehicle_id = cell->vehicle_id;
    CellType  type       = cell->type;
    Direction dir        = cell->allowed_dir;
    int       inter_id   = cell->intersection_id;
    pthread_mutex_unlock(&cell->lock);

    /* 1. Veículo tem prioridade visual sobre tudo. */
    const Vehicle *v = find_vehicle(vehicles, n_vehicles, vehicle_id);
    if (occupied && v != NULL) {
        printf("%c ", vehicle_symbol(v->type));
        return;
    }

    /* 2. Cruzamento vazio -> mostra estado do semáforo (lido sob o
     * mutex do próprio semáforo, escrito por traffic_light_thread /
     * request_priority). */
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

    /* 3. Pista comum -> seta da direção. */
    if (type == CELL_ROAD) {
        printf("%s", road_arrow(dir));
        return;
    }

    /* 4. Parede / calçada. */
    printf("  "); /* ██ */
}

void render_map(Map *m, Vehicle *vehicles, int n_vehicles, TrafficLight *lights) {
    /* Limpa a tela do terminal a cada tick (sem dependência de ncurses). */
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
