#include "platform.h"
#include <stdio.h>

GLFWwindow *window = nullptr;

GLFWwindow* platform_get_window() {
	return window;
}

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


const char **platform_get_required_extensions(uint32_t *out_count) {
	return glfwGetRequiredInstanceExtensions(out_count);
}

bool platform_create_vulkan_surface(VkInstance instance, VkSurfaceKHR *out_surface) {
	if (window == NULL || instance == VK_NULL_HANDLE || out_surface == NULL) {
		return false;
	}
	return glfwCreateWindowSurface(instance, window, NULL, out_surface) == VK_SUCCESS;
}
