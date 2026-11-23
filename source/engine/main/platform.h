#ifndef HOLLOWENGINE_PLATFORM_HEADER
#define HOLLOWENGINE_PLATFORM_HEADER

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct WindowsSpecification {
	const char *title;
	int Width;
	int Height;
} WindowsSpecification;

extern WindowsSpecification *WinSpecs;
extern GLFWwindow *window;

bool platform_init(WindowsSpecification *WindowSpecs);
void platform_update();
void platform_shutdown();

bool platform_shouldclose();

#endif
