
#include "main/time.h"

#include <GLFW/glfw3.h>

static EngineTime engine_time = {0};

void time_init(void) {
    glfwSetTime(0.0);

    engine_time.delta_time = 0.0;
    engine_time.elapsed_time = 0.0;
    engine_time.frame_rate = 0.0;
}

void time_update(void) {
    double current_time = glfwGetTime();

    engine_time.delta_time =
        current_time - engine_time.elapsed_time;

    engine_time.elapsed_time = current_time;

    if (engine_time.delta_time > 0.0) {
        engine_time.frame_rate = 1.0 / engine_time.delta_time;
    }
}

double time_get_delta(void) {
    return engine_time.delta_time;
}

double time_get_elapsed(void) {
    return engine_time.elapsed_time;
}

double time_get_fps(void) {
    return engine_time.frame_rate;
}
