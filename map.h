#ifndef MAP_H
#define MAP_H

#include <pthread.h>

typedef enum {
    CELL_WALL,         
    CELL_ROAD,          
    CELL_INTERSECTION   
} CellType;

typedef enum {
    DIR_RIGHT,  
    DIR_LEFT, 
    DIR_UP,    
    DIR_DOWN,   
    DIR_NONE    
} Direction;

typedef struct {
    CellType   type;
    Direction  allowed_dir;     
    int        occupied;       
    int        vehicle_id;      
    int        intersection_id;  
    pthread_mutex_t lock;       
} Cell;


#define MAP_ROWS 21
#define MAP_COLS 55
#define NUM_INTERSECTIONS 8

typedef struct {
    Cell grid[MAP_ROWS][MAP_COLS];
} Map;

extern Map g_map;

void map_init(Map *m);

void map_destroy(Map *m);

int map_in_bounds(int row, int col);

const char *direction_name(Direction d);

#endif
