#include "vulkan.h"
#include "../utils/vector.h"

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkInstance instance = VK_NULL_HANDLE;

static bool checkExtensionSupport(const char **requiredExtensions,
								  uint32_t requiredCount) {
	uint32_t extensionCount = 0;

	VkResult result =
		vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	if (result != VK_SUCCESS) {
		printf("Failed to enumerate Vulkan extensions: %d\n", result);
		return false;
	}

	// Automatically calls vector_free(&availableExtensions) on every return path!
	VECTOR_AUTO availableExtensions = VECTOR_INIT(VkExtensionProperties);
	vector_resize(&availableExtensions, extensionCount);

	if (availableExtensions.data == NULL && extensionCount > 0) {
		printf("Failed to allocate extension memory\n");
		return false;
	}

	result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
													availableExtensions.data);

	if (result != VK_SUCCESS) {
		printf("Failed to retrieve Vulkan extensions: %d\n", result);
		return false; // No manual free needed!
	}

	printf("\nAvailable Vulkan extensions:\n");

	for (size_t i = 0; i < availableExtensions.length; i++) {
		VkExtensionProperties ext =
			VECTOR_GET(availableExtensions, VkExtensionProperties, i);
		printf("  %s\n", ext.extensionName);
	}

	bool supported = true;

	for (uint32_t i = 0; i < requiredCount; i++) {
		bool found = false;

		for (size_t j = 0; j < availableExtensions.length; j++) {
			VkExtensionProperties ext =
				VECTOR_GET(availableExtensions, VkExtensionProperties, j);

			if (strcmp(requiredExtensions[i], ext.extensionName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			printf("Missing required Vulkan extension: %s\n",
				   requiredExtensions[i]);
			supported = false;
		}
	}

	return supported; // vector_free runs automatically right here
}

void createInstance(void) {
	VkApplicationInfo appInfo = {0};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HollowEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "HollowEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t glfwExtensionCount = 0;

	const char **glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (glfwExtensions == NULL || glfwExtensionCount == 0) {
		printf("Failed to get GLFW Vulkan extensions\n");
		return;
	}

	if (!checkExtensionSupport(glfwExtensions, glfwExtensionCount)) {
		printf("Required Vulkan extensions are unavailable\n");
		return;
	}

	VkInstanceCreateInfo createInfo = {0};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, NULL, &instance);

	if (result != VK_SUCCESS) {
		printf("Failed to create Vulkan instance: %d\n", result);
		instance = VK_NULL_HANDLE;
		return;
	}

	printf("\nVulkan Instance Created Successfully!\n");
}

void initVulkan(void) {
	createInstance();

	if (instance == VK_NULL_HANDLE) {
		printf("Vulkan initialization failed\n");
		return;
	}
}

void cleanupVulkan(void) {
	if (instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, NULL);
		instance = VK_NULL_HANDLE;

		printf("Vulkan Instance Destroyed\n");
	}
}
