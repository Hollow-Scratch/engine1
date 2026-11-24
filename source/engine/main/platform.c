#include "platform.h"
#include <stdio.h>

GLFWwindow *window = nullptr;

GLFWwindow *platformGetWindow(void) { return window; }

bool platformInit(WindowSpecification *windowSpecs) {

	if (!glfwInit()) {
		printf("GLFW init error\n");
		return -1;
	};
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(windowSpecs->width, windowSpecs->height,
							  windowSpecs->title, NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}
	return true;
}

void platformUpdate(void) { glfwPollEvents(); }

void platformShutdown(void) { glfwTerminate(); }

bool platformShouldClose(void) { return glfwWindowShouldClose(window); }

const char **platformGetRequiredExtensions(uint32_t *outCount) {
	return glfwGetRequiredInstanceExtensions(outCount);
}

bool platformCreateVulkanSurface(VkInstance instance,
								 VkSurfaceKHR *outSurface) {
	if (window == NULL || instance == VK_NULL_HANDLE || outSurface == NULL) {
		return false;
	}
	return glfwCreateWindowSurface(instance, window, NULL, outSurface) ==
		   VK_SUCCESS;
}
