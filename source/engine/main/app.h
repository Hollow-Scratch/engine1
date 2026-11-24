#pragma once

#include <stdbool.h>

extern bool isRunning;

void initializeApp(void);
void updateApp(double deltaTime);
void shutdownApp(void);
