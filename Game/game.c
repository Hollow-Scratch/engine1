#include <stdio.h>
#include "hollowengine.h"

void HOLLOWENGINE_INIT(void) {

}

void HOLLOWENGINE_UPDATE(double delta_time) {
    printf(" 1... 2... 3... Testing Hello! delta is %f\n", delta_time);
}

void HOLLOWENGINE_SHUTDOWN(void) {
    printf("g");
}
