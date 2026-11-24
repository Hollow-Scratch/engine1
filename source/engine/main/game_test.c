#include "app.h"

#include <stdio.h>

void initializeApp(void) {}

void updateApp(double deltaTime) {
	(void)deltaTime;
	printf(" 1... 2... 3... Testing Hello! delta is %f\n", deltaTime);
}

void shutdownApp(void) {}
