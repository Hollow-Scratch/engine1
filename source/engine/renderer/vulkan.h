#pragma once

#include <vulkan/vulkan_core.h>

extern VkInstance instance;

void initializeVulkan(void);
void createInstance(void);
void cleanupVulkan(void);
void drawVulkanFrame(void);
