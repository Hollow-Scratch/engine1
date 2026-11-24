#pragma once

#include <vulkan/vulkan_core.h>

extern VkInstance g_instance;

void InitializeVulkan(void);
void CreateInstance(void);
void CleanupVulkan(void);
void DrawVulkanFrame(void);
