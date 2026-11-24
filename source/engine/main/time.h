
#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

typedef struct {
	double delta_time;
	double elapsed_time;
	double frame_rate;
} EngineTime;

void TimeInit(void);
void TimeUpdate(void);

double TimeGetDelta(void);
double TimeGetElapsed(void);
double TimeGetFps(void);

#endif
