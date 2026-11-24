#pragma once

#include <stdbool.h>

extern bool isRunning;

void initializeApp(void);
void updateApp(float deltaTime);
void shutdownApp(void);
