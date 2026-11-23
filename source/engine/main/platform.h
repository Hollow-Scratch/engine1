#ifndef HOLLOWENGINE_PLATFORM_HEADER
#define HOLLOWENGINE_PLATFORM_HEADER

#include <GLFW/glfw3.h>

typedef struct WindowsSpecification {
  const char *title;
  int Width;
  int Height;
} WindowsSpecification;

extern WindowsSpecification *WinSpecs;
GLFWwindow *window = nullptr;

bool platform_init(WindowsSpecification *WindowSpecs);
void platform_update();
void platform_shutdown();

#endif
