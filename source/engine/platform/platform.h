#pragma once

#include <stdbool.h>
#include <stdint.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct WindowSpecification {
	const char *title;
	int			width;
	int			height;
} WindowSpecification;

extern GLFWwindow *g_window;

GLFWwindow	*PlatformGetWindow(void);
bool		 PlatformInit(WindowSpecification *window_specs);
void		 PlatformUpdate(void);
void		 PlatformShutdown(void);
bool		 PlatformShouldClose(void);
const char **PlatformGetRequiredExtensions(uint32_t *out_count);
bool		 PlatformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *out_surface);
