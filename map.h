#ifndef MAP_H
#define MAP_H

#include <pthread.h>

/* =======================================================================
 * INTEGRANTE 1 - Mapa, estruturas de dados e visualização ASCII
 * Arquivo: map.h
 *
 * Define os tipos base compartilhados por todo o projeto: tipo de célula,
 * direção permitida e a matriz que representa a malha viária.
 * ===================================================================== */

typedef enum {
    CELL_WALL,          /* ████ calçada/parede, célula intransitável      */
    CELL_ROAD,          /* célula de pista normal                         */
    CELL_INTERSECTION   /* célula de cruzamento (controlada por semáforo) */
} CellType;

typedef enum {
    DIR_RIGHT,   /* -> */
    DIR_LEFT,    /* <- */
    DIR_UP,      /* ^  */
    DIR_DOWN,    /* v  */
    DIR_NONE     /* parede / sem direção definida */
} Direction;

typedef struct {
    CellType   type;
    Direction  allowed_dir;      /* direção permitida nesta célula          */
    int        occupied;         /* 0 = livre, 1 = ocupado                  */
    int        vehicle_id;       /* id do veículo presente (-1 se livre)    */
    int        intersection_id;  /* -1 se não for cruzamento, 0..7 se for   */
    pthread_mutex_t lock;        /* mutex individual da célula (exclusão
                                     mútua granular: só quem disputa a MESMA
                                     célula se bloqueia)                    */
} Cell;

/* Dimensões da malha viária. O layout implementado em map.c usa:
 *   - 4 ruas horizontais (2 mão dupla + 2 mão única)
 *   - 2 vias verticais de mão dupla
 *   - 8 cruzamentos (4 ruas x 2 vias verticais)
 */
#define MAP_ROWS 21
#define MAP_COLS 55
#define NUM_INTERSECTIONS 8

typedef struct {
    Cell grid[MAP_ROWS][MAP_COLS];
} Map;

/* Instância global do mapa, definida em main.c (Integrante 3) e usada
 * pelas threads de veículo para consultar/alterar células. */
extern Map g_map;

/* Inicializa a malha viária completa: paredes, pistas, direções e
 * cruzamentos, além de inicializar o mutex de cada célula. */
void map_init(Map *m);

/* Destrói os mutexes de todas as células (chamar ao encerrar a simulação). */
void map_destroy(Map *m);

/* Retorna 1 se (row, col) está dentro dos limites do mapa, 0 caso contrário. */
int map_in_bounds(int row, int col);

/* Utilitário: nome legível de uma direção (para logs/depuração). */
const char *direction_name(Direction d);

#endif /* MAP_H */
