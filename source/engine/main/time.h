
#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

typedef struct {
    double delta_time;
    double elapsed_time;
    double frame_rate;
} EngineTime;

void time_init(void);
void time_update(void);

double time_get_delta(void);
double time_get_elapsed(void);
double time_get_fps(void);

#endif
