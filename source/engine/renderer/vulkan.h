#ifndef HOLLOWENGINE_VULKAN_HEADER
#define HOLLOWENGINE_VULKAN_HEADER

#include <vulkan/vulkan_core.h>

extern VkInstance instance;

void initVulkan();
void createInstance();
void cleanupVulkan();
void drawVulkanFrame();

#endif
