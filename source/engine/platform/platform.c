#include "platform.h"

#include <stdio.h>

GLFWwindow *g_window = nullptr;

GLFWwindow *PlatformGetWindow(void) {
	return g_window;
}

bool PlatformInit(WindowSpecification *window_specs) {
	if (!glfwInit()) {
		printf("GLFW init error\n");
		return -1;
	};
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	g_window = glfwCreateWindow(window_specs->width, window_specs->height, window_specs->title,
								NULL, NULL);

	if (!g_window) {
		glfwTerminate();
		return -1;
	}
	return true;
}

void PlatformUpdate(void) {
	glfwPollEvents();
}

void PlatformShutdown(void) {
	glfwTerminate();
}

bool PlatformShouldClose(void) {
	return glfwWindowShouldClose(g_window);
}

const char **PlatformGetRequiredExtensions(uint32_t *out_count) {
	return glfwGetRequiredInstanceExtensions(out_count);
}

bool PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *out_surface) {
	if (g_window == NULL || instance == VK_NULL_HANDLE || out_surface == NULL) {
		return false;
	}
	return glfwCreateWindowSurface(instance, g_window, NULL, out_surface) == VK_SUCCESS;
}
