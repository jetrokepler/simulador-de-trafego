#ifndef VEHICLE_H
#define VEHICLE_H

#include "map.h"

typedef enum {
    CAR_FAST, 
    CAR_MEDIUM,
    CAR_SLOW,  
    AMBULANCE  
} VehicleType;

typedef struct {
    int         id;
    VehicleType type;
    int         row, col;   
    Direction   dir;        
    int         speed;     
    int        *route;      
    int         route_len;
    int         route_idx; 
    int         active;  
} Vehicle;


#define NUM_VEHICLES 15


void vehicles_init_all(Vehicle vehicles[], int n);


void vehicles_destroy_all(Vehicle vehicles[], int n);


void *vehicle_thread(void *arg);

void try_move(Vehicle *v);

int is_valid_move(int nr, int nc, Direction dir);

#endif
