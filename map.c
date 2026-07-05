#include <stdio.h>
#include "map.h"

/* =======================================================================
 * INTEGRANTE 1 - Mapa, estruturas de dados e visualização ASCII
 * Arquivo: map.c
 *
 * Layout implementado (consistente com mapa.md e atividades.md):
 *
 *   - 4 ruas horizontais:
 *       band 0 -> mão dupla superior (linha 3 = "->", linha 4 = "<-")
 *       band 1 -> mão única 1        (linha 8 = "->")
 *       band 2 -> mão única 2        (linha 12 = "->")
 *       band 3 -> mão dupla inferior (linha 16 = "->", linha 17 = "<-")
 *
 *   - 2 vias verticais de mão dupla:
 *       vertical esquerda -> coluna 15 = "^" (DIR_UP), coluna 16 = "v" (DIR_DOWN)
 *       vertical direita  -> coluna 38 = "^" (DIR_UP), coluna 39 = "v" (DIR_DOWN)
 *
 *   - 8 cruzamentos = 4 bandas horizontais x 2 vias verticais.
 *     intersection_id = band_index * 2 + side  (side: 0 = esquerda, 1 = direita)
 *     Isso reproduz exatamente a tabela do plano de ação (IDs 0..7).
 * ===================================================================== */

typedef struct {
    int       row;
    Direction dir;
    int       band_index;
} HBand;

typedef struct {
    int       col;
    Direction dir;
    int       side;
} VCol;

/* Bandas horizontais: linhas de pista e sua direção/índice de rua. */
static const HBand H_BANDS[] = {
    {3,  DIR_RIGHT, 0},   /* mão dupla superior - faixa de ida   */
    {4,  DIR_LEFT,  0},   /* mão dupla superior - faixa de volta */
    {8,  DIR_RIGHT, 1},   /* mão única 1                         */
    {12, DIR_RIGHT, 2},   /* mão única 2                         */
    {16, DIR_RIGHT, 3},   /* mão dupla inferior - faixa de ida   */
    {17, DIR_LEFT,  3},   /* mão dupla inferior - faixa de volta */
};
#define N_HBANDS (int)(sizeof(H_BANDS) / sizeof(H_BANDS[0]))

/* Colunas verticais: mão dupla, uma coluna para cada sentido. */
static const VCol V_COLS[] = {
    {15, DIR_UP,   0},  /* vertical esquerda - sobe */
    {16, DIR_DOWN, 0},  /* vertical esquerda - desce */
    {38, DIR_UP,   1},  /* vertical direita - sobe */
    {39, DIR_DOWN, 1},  /* vertical direita - desce */
};
#define N_VCOLS (int)(sizeof(V_COLS) / sizeof(V_COLS[0]))

static const HBand *find_hband(int row) {
    for (int i = 0; i < N_HBANDS; i++)
        if (H_BANDS[i].row == row) return &H_BANDS[i];
    return NULL;
}

static const VCol *find_vcol(int col) {
    for (int i = 0; i < N_VCOLS; i++)
        if (V_COLS[i].col == col) return &V_COLS[i];
    return NULL;
}

int map_in_bounds(int row, int col) {
    return row >= 0 && row < MAP_ROWS && col >= 0 && col < MAP_COLS;
}

const char *direction_name(Direction d) {
    switch (d) {
        case DIR_RIGHT: return "RIGHT";
        case DIR_LEFT:  return "LEFT";
        case DIR_UP:    return "UP";
        case DIR_DOWN:  return "DOWN";
        default:        return "NONE";
    }
}

void map_init(Map *m) {
    for (int r = 0; r < MAP_ROWS; r++) {
        const HBand *hb = find_hband(r);

        for (int c = 0; c < MAP_COLS; c++) {
            const VCol *vc = find_vcol(c);
            Cell *cell = &m->grid[r][c];

            cell->occupied        = 0;
            cell->vehicle_id      = -1;
            cell->intersection_id = -1;

            if (hb != NULL && vc != NULL) {
                /* Cruzamento: rua horizontal encontra via vertical.
                 * O semáforo horizontal (tl->state_horiz) controla a
                 * entrada de veículos que atravessam esta célula pela
                 * rua horizontal; o semáforo vertical (tl->state_vert)
                 * controla quem atravessa pela via vertical no mesmo
                 * cruzamento (ver traffic_light.h do Integrante 2). */
                cell->type            = CELL_INTERSECTION;
                cell->allowed_dir     = hb->dir;
                cell->intersection_id = hb->band_index * 2 + vc->side;
            } else if (hb != NULL) {
                /* Pista horizontal comum. */
                cell->type        = CELL_ROAD;
                cell->allowed_dir = hb->dir;
            } else if (vc != NULL) {
                /* Pista vertical comum. */
                cell->type        = CELL_ROAD;
                cell->allowed_dir = vc->dir;
            } else {
                /* Calçada / parede. */
                cell->type        = CELL_WALL;
                cell->allowed_dir = DIR_NONE;
            }

            pthread_mutex_init(&cell->lock, NULL);
        }
    }
}

void map_destroy(Map *m) {
    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++)
            pthread_mutex_destroy(&m->grid[r][c].lock);
}
