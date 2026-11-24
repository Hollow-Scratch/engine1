
#include "main/time.h"

#include <GLFW/glfw3.h>

static EngineTime g_engine_time = {0};

void TimeInit(void) {
	glfwSetTime(0.0);

	g_engine_time.delta_time   = 0.0;
	g_engine_time.elapsed_time = 0.0;
	g_engine_time.frame_rate   = 0.0;
}

void TimeUpdate(void) {
	double current_time = glfwGetTime();

	g_engine_time.delta_time = current_time - g_engine_time.elapsed_time;

	g_engine_time.elapsed_time = current_time;

	if (g_engine_time.delta_time > 0.0) {
		g_engine_time.frame_rate = 1.0 / g_engine_time.delta_time;
	}
}

double TimeGetDelta(void) {
	return g_engine_time.delta_time;
}

double TimeGetElapsed(void) {
	return g_engine_time.elapsed_time;
}

double TimeGetFps(void) {
	return g_engine_time.frame_rate;
}
