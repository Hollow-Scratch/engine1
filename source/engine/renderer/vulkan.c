#include "vulkan.h"
#include "../main/platform.h"
#include "../utils/vector.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
static VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

static const char *validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};
static const uint32_t validationLayerCount = 1;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData) {
	(void)messageSeverity;
	(void)messageType;
	(void)pUserData;

	fprintf(stderr, "[Vulkan Validation]: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
	*createInfo = (VkDebugUtilsMessengerCreateInfoEXT){
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
	};
}

static void setupDebugMessenger(void) {
	if (!enableValidationLayers) {
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(&createInfo);

	PFN_vkCreateDebugUtilsMessengerEXT func =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != NULL) {
		func(instance, &createInfo, NULL, &debugMessenger);
	}
}

static bool checkValidationLayerSupport(void) {
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VECTOR_AUTO availableLayers = VECTOR_INIT(VkLayerProperties);
	vector_resize(&availableLayers, layerCount);

	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data);

	for (uint32_t i = 0; i < validationLayerCount; i++) {
		bool found = false;

		for (size_t j = 0; j < availableLayers.length; j++) {
			VkLayerProperties layer =
				VECTOR_GET(availableLayers, VkLayerProperties, j);

			if (strcmp(validationLayers[i], layer.layerName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}
	}

	return true;
}

static bool checkExtensionSupport(const char **requiredExtensions,
								  uint32_t requiredCount) {
	uint32_t extensionCount = 0;

	VkResult result =
		vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	if (result != VK_SUCCESS) {
		printf("Failed to enumerate Vulkan extensions: %d\n", result);
		return false;
	}

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
		return false;
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

	return supported;
}

void createInstance(void) {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		printf("Validation layers requested, but not available!\n");
		return;
	}

	VkApplicationInfo appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HollowEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "HollowEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t platformExtensionCount = 0;
	const char **platformExtensions =
		platform_get_required_extensions(&platformExtensionCount);

	if (platformExtensions == NULL || platformExtensionCount == 0) {
		printf("Failed to get platform Vulkan extensions\n");
		return;
	}

	VECTOR_AUTO extensions = VECTOR_INIT(const char *);
	for (uint32_t i = 0; i < platformExtensionCount; i++) {
		vector_append(&extensions, &platformExtensions[i]);
	}
	if (enableValidationLayers) {
		const char *debugExt = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		vector_append(&extensions, &debugExt);
	}

	if (!checkExtensionSupport(extensions.data, (uint32_t)extensions.length)) {
		printf("Required Vulkan extensions are unavailable\n");
		return;
	}

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = (uint32_t)extensions.length;
	createInfo.ppEnabledExtensionNames = extensions.data;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = validationLayerCount;
		createInfo.ppEnabledLayerNames = validationLayers;

		populateDebugMessengerCreateInfo(&debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = NULL;
	}

	VkResult result = vkCreateInstance(&createInfo, NULL, &instance);

	if (result != VK_SUCCESS) {
		printf("Failed to create Vulkan instance: %d\n", result);
		instance = VK_NULL_HANDLE;
		return;
	}

	setupDebugMessenger();
	printf("\nVulkan Instance Created Successfully!\n");
}

void initVulkan(void) {
	createInstance();
	if (instance == VK_NULL_HANDLE) {
		printf("Vulkan initialization failed\n");
		return;
	}

	if (!platform_create_vulkan_surface(instance, &surface)) {
		printf("Failed to create Vulkan window surface!\n");
		return;
	}

	printf("Vulkan Surface Created Successfully!\n");
}

void cleanupVulkan(void) {
	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(instance, surface, NULL);
		surface = VK_NULL_HANDLE;
		printf("Vulkan Surface Destroyed\n");
	}

	if (enableValidationLayers && debugMessenger != VK_NULL_HANDLE) {
		PFN_vkDestroyDebugUtilsMessengerEXT func =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != NULL) {
			func(instance, debugMessenger, NULL);
			debugMessenger = VK_NULL_HANDLE;
		}
	}

	if (instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, NULL);
		instance = VK_NULL_HANDLE;
		printf("Vulkan Instance Destroyed\n");
	}
}
