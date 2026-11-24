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

typedef struct VkInstance_T *VkInstance;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

GLFWwindow *platform_get_window();

bool platform_init(WindowsSpecification *WindowSpecs);
void platform_update();
void platform_shutdown();

bool platform_shouldclose();

const char **platform_get_required_extensions(uint32_t *out_count);
bool platform_create_vulkan_surface(VkInstance instance, VkSurfaceKHR *out_surface);

#endif
