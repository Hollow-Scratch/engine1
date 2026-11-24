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

extern GLFWwindow *window;

GLFWwindow	*platformGetWindow(void);
bool		 platformInit(WindowSpecification *windowSpecs);
void		 platformUpdate(void);
void		 platformShutdown(void);
bool		 platformShouldClose(void);
const char **platformGetRequiredExtensions(uint32_t *outCount);
bool		 platformCreateVulkanSurface(VkInstance instance, VkSurfaceKHR *outSurface);
