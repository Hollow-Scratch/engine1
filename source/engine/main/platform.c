#include "platform.h"
#include <stdio.h>

GLFWwindow *window = nullptr;

bool platform_init(WindowsSpecification *WindowsSpecs) {

	if (!glfwInit()) {
		printf("GLFW init error\n");
		return -1;
	};
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WindowsSpecs->Width, WindowsSpecs->Height,
							  WindowsSpecs->title, NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}
	return true;
}

void platform_update() { glfwPollEvents(); }

void platform_shutdown() { glfwTerminate(); }

bool platform_shouldclose() { return glfwWindowShouldClose(window); }
