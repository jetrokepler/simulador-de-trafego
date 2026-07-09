#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "map.h"
#include "clock.h"
#include "traffic_light.h"
#include "render.h"


TrafficLight lights[NUM_INTERSECTIONS];

static Map map;

int main(void) {
    map_init(&map);
    clock_init();
    lights_init(lights, NUM_INTERSECTIONS);

    pthread_t clock_tid;
    pthread_t light_tids[NUM_INTERSECTIONS];

    pthread_create(&clock_tid, NULL, clock_thread, NULL);
    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_create(&light_tids[i], NULL, traffic_light_thread, &lights[i]);

 
    for (long t = 1; t <= 40; t++) {
        wait_until_tick(t);
        render_map(&map, NULL, 0, lights);
        printf("tick = %ld\n", t);
    }


    simulation_running = 0;
    pthread_mutex_lock(&g_clock.lock);
    pthread_cond_broadcast(&g_clock.cond);
    pthread_mutex_unlock(&g_clock.lock);

    pthread_join(clock_tid, NULL);
    for (int i = 0; i < NUM_INTERSECTIONS; i++)
        pthread_join(light_tids[i], NULL);

    lights_destroy(lights, NUM_INTERSECTIONS);
    clock_destroy();
    map_destroy(&map);

    printf("Demo 2 encerrada sem travar. OK.\n");
    return 0;
}
