#include <stdio.h>
#include "map.h"


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
                
                cell->type            = CELL_INTERSECTION;
                cell->allowed_dir     = hb->dir;
                cell->intersection_id = hb->band_index * 2 + vc->side;
            } else if (hb != NULL) {
           
                cell->type        = CELL_ROAD;
                cell->allowed_dir = hb->dir;
            } else if (vc != NULL) {
               
                cell->type        = CELL_ROAD;
                cell->allowed_dir = vc->dir;
            } else {
           
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
