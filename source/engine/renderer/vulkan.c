#include "vulkan.h"
#include "../main/platform.h"
#include "../utils/vector.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;
VkQueue graphicsQueue = VK_NULL_HANDLE;
VkQueue presentQueue = VK_NULL_HANDLE;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
VkFormat swapchainImageFormat;
VkExtent2D swapchainExtent;
vector swapchainImages = {0};	  // Stores VkImage handles
vector swapchainImageViews = {0}; // Stores VkImageView handles
VkCommandPool commandPool = VK_NULL_HANDLE;
VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
VkFence inFlightFence = VK_NULL_HANDLE;
VkRenderPass renderPass = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkPipeline graphicsPipeline = VK_NULL_HANDLE;
vector swapchainFramebuffers = {0}; // Stores VkFramebuffer handles

static VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

static const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
static const uint32_t validationLayerCount = 1;

static const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const uint32_t deviceExtensionCount = 1;

typedef struct {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	bool hasGraphics;
	bool hasPresent;
} QueueFamilyIndices;

static bool isQueueComplete(QueueFamilyIndices indices) {
	return indices.hasGraphics && indices.hasPresent;
}

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice targetDevice) {
	QueueFamilyIndices indices = {0};

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(targetDevice, &queueFamilyCount,
											 NULL);

	VECTOR_AUTO queueFamilies = VECTOR_INIT(VkQueueFamilyProperties);
	vector_resize(&queueFamilies, queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(targetDevice, &queueFamilyCount,
											 queueFamilies.data);

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		VkQueueFamilyProperties queueFamily =
			VECTOR_GET(queueFamilies, VkQueueFamilyProperties, i);

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			indices.hasGraphics = true;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(targetDevice, i, surface,
											 &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
			indices.hasPresent = true;
		}

		if (isQueueComplete(indices)) {
			break;
		}
	}

	return indices;
}

static bool checkDeviceExtensionSupport(VkPhysicalDevice targetDevice) {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(targetDevice, NULL, &extensionCount,
										 NULL);

	VECTOR_AUTO availableExtensions = VECTOR_INIT(VkExtensionProperties);
	vector_resize(&availableExtensions, extensionCount);
	vkEnumerateDeviceExtensionProperties(targetDevice, NULL, &extensionCount,
										 availableExtensions.data);

	for (uint32_t i = 0; i < deviceExtensionCount; i++) {
		bool found = false;
		for (size_t j = 0; j < availableExtensions.length; j++) {
			VkExtensionProperties ext =
				VECTOR_GET(availableExtensions, VkExtensionProperties, j);

			if (strcmp(deviceExtensions[i], ext.extensionName) == 0) {
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

static bool isDeviceSuitable(VkPhysicalDevice targetDevice) {
	QueueFamilyIndices indices = findQueueFamilies(targetDevice);
	bool extensionsSupported = checkDeviceExtensionSupport(targetDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(targetDevice, surface,
											 &formatCount, NULL);

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(targetDevice, surface,
												  &presentModeCount, NULL);

		swapChainAdequate = (formatCount > 0) && (presentModeCount > 0);
	}

	return isQueueComplete(indices) && extensionsSupported && swapChainAdequate;
}

static void pickPhysicalDevice(void) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

	if (deviceCount == 0) {
		printf("Failed to find GPUs with Vulkan support!\n");
		return;
	}

	VECTOR_AUTO devices = VECTOR_INIT(VkPhysicalDevice);
	vector_resize(&devices, deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data);

	printf("\nDetected Vulkan GPUs:\n");
	VkPhysicalDevice fallbackDevice = VK_NULL_HANDLE;

	for (size_t i = 0; i < devices.length; i++) {
		VkPhysicalDevice candidate = VECTOR_GET(devices, VkPhysicalDevice, i);

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(candidate, &props);
		printf("  [%zu] %s\n", i, props.deviceName);

		if (isDeviceSuitable(candidate)) {
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				physicalDevice = candidate;
				break;
			}
			if (fallbackDevice == VK_NULL_HANDLE) {
				fallbackDevice = candidate;
			}
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		physicalDevice = fallbackDevice;
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		printf("Failed to find a suitable GPU!\n");
		return;
	}

	VkPhysicalDeviceProperties selectedProps;
	vkGetPhysicalDeviceProperties(physicalDevice, &selectedProps);
	printf("Selected GPU: %s\n", selectedProps.deviceName);
}

static void createLogicalDevice(void) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	VECTOR_AUTO uniqueQueueFamilies = VECTOR_INIT(uint32_t);
	vector_append(&uniqueQueueFamilies, &indices.graphicsFamily);
	if (indices.presentFamily != indices.graphicsFamily) {
		vector_append(&uniqueQueueFamilies, &indices.presentFamily);
	}

	VECTOR_AUTO queueCreateInfos = VECTOR_INIT(VkDeviceQueueCreateInfo);
	float queuePriority = 1.0f;

	for (size_t i = 0; i < uniqueQueueFamilies.length; i++) {
		uint32_t familyIndex = VECTOR_GET(uniqueQueueFamilies, uint32_t, i);

		VkDeviceQueueCreateInfo queueCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = familyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};
		vector_append(&queueCreateInfos, &queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {0};

	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = (uint32_t)queueCreateInfos.length,
		.pQueueCreateInfos = queueCreateInfos.data,
		.pEnabledFeatures = &deviceFeatures,
		.enabledExtensionCount = deviceExtensionCount,
		.ppEnabledExtensionNames = deviceExtensions,
	};

	if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) !=
		VK_SUCCESS) {
		printf("Failed to create logical device!\n");
		device = VK_NULL_HANDLE;
		return;
	}

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);

	printf("Vulkan Logical Device & Queues Created Successfully!\n");
}

static uint32_t clamp_u32(uint32_t value, uint32_t min, uint32_t max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

static void createSwapchain(uint32_t width, uint32_t height) {
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
											  &capabilities);

	// 1. Pick Surface Format (prefer SRGB)
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
										 NULL);
	VECTOR_AUTO formats = VECTOR_INIT(VkSurfaceFormatKHR);
	vector_resize(&formats, formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
										 formats.data);

	VkSurfaceFormatKHR surfaceFormat =
		VECTOR_GET(formats, VkSurfaceFormatKHR, 0);
	for (size_t i = 0; i < formats.length; i++) {
		VkSurfaceFormatKHR fmt = VECTOR_GET(formats, VkSurfaceFormatKHR, i);
		if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
			fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surfaceFormat = fmt;
			break;
		}
	}

	// 2. Pick Present Mode (FIFO is guaranteed on all platforms including
	// Wayland)
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	// 3. Pick Swap Extent (Wayland sets currentExtent to UINT32_MAX, so we
	// clamp our window size)
	VkExtent2D extent = capabilities.currentExtent;
	if (extent.width == UINT32_MAX) {
		extent.width = clamp_u32(width, capabilities.minImageExtent.width,
								 capabilities.maxImageExtent.width);
		extent.height = clamp_u32(height, capabilities.minImageExtent.height,
								  capabilities.maxImageExtent.height);
	}

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 &&
		imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					  VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily,
									 indices.presentFamily};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain) !=
		VK_SUCCESS) {
		printf("Failed to create swapchain!\n");
		swapchain = VK_NULL_HANDLE;
		return;
	}

	// Retrieve the swapchain VkImage handles into our long-lived vector
	uint32_t actualImageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &actualImageCount, NULL);
	swapchainImages = VECTOR_INIT(VkImage);
	vector_resize(&swapchainImages, actualImageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &actualImageCount,
							swapchainImages.data);

	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;

	printf("Vulkan Swapchain Created (%ux%u, %u images)!\n", extent.width,
		   extent.height, actualImageCount);
}

static void createImageViews(void) {
	swapchainImageViews = VECTOR_INIT(VkImageView);
	vector_resize(&swapchainImageViews, swapchainImages.length);

	for (size_t i = 0; i < swapchainImages.length; i++) {
		VkImage image = VECTOR_GET(swapchainImages, VkImage, i);

		VkImageViewCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchainImageFormat,
			.components =
				{
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY,
				},
			.subresourceRange =
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
		};

		VkImageView imageView = VK_NULL_HANDLE;
		if (vkCreateImageView(device, &createInfo, NULL, &imageView) !=
			VK_SUCCESS) {
			printf("Failed to create image view %zu!\n", i);
			return;
		}
		VECTOR_GET(swapchainImageViews, VkImageView, i) = imageView;
	}

	printf("Vulkan Image Views Created (%zu views)!\n",
		   swapchainImageViews.length);
}

static void createSyncAndCommands(void) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = indices.graphicsFamily,
	};
	vkCreateCommandPool(device, &poolInfo, NULL, &commandPool);

	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkSemaphoreCreateInfo semInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	vkCreateSemaphore(device, &semInfo, NULL, &imageAvailableSemaphore);
	vkCreateSemaphore(device, &semInfo, NULL, &renderFinishedSemaphore);
	vkCreateFence(device, &fenceInfo, NULL, &inFlightFence);
}

void drawVulkanFrame(void) {
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
						  imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffer, 0);
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkClearValue clearColor = {{{0.05f, 0.05f, 0.08f, 1.0f}}};
	VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass,
		.framebuffer =
			VECTOR_GET(swapchainFramebuffers, VkFramebuffer, imageIndex),
		.renderArea = {.offset = {0, 0}, .extent = swapchainExtent},
		.clearValueCount = 1,
		.pClearValues = &clearColor,
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
						 VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					  graphicsPipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0,
			  0); // Draws the 3 vertices of your triangle!
	vkCmdEndRenderPass(commandBuffer);

	vkEndCommandBuffer(commandBuffer);

	VkPipelineStageFlags waitStage =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &imageAvailableSemaphore,
		.pWaitDstStageMask = &waitStage,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderFinishedSemaphore,
	};
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
	};
	vkQueuePresentKHR(presentQueue, &presentInfo);
}

static void createRenderPass(void) {
	VkAttachmentDescription colorAttachment = {
		.format = swapchainImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) !=
		VK_SUCCESS) {
		printf("Failed to create render pass!\n");
	}
}

static VkShaderModule createShaderModule(const char *filepath) {
	FILE *file = fopen(filepath, "rb");
	if (!file) {
		printf("Failed to open shader file: %s\n", filepath);
		return VK_NULL_HANDLE;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char *buffer = malloc(fileSize);
	fread(buffer, 1, fileSize, file);
	fclose(file);

	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = (size_t)fileSize,
		.pCode = (const uint32_t *)buffer,
	};

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) !=
		VK_SUCCESS) {
		printf("Failed to create shader module for %s!\n", filepath);
	}

	free(buffer);
	return shaderModule;
}

static void createGraphicsPipeline(void) {
	VkShaderModule vertShaderModule = createShaderModule("shaders/vert.spv");
	VkShaderModule fragShaderModule = createShaderModule("shaders/frag.spv");
	if (vertShaderModule == VK_NULL_HANDLE ||
		fragShaderModule == VK_NULL_HANDLE) {
		return;
	}

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
			.pName = "main",
		},
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)swapchainExtent.width,
		.height = (float)swapchainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = swapchainExtent,
	};

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	};
	vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0,
	};

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
								  NULL, &graphicsPipeline) != VK_SUCCESS) {
		printf("Failed to create graphics pipeline!\n");
	}

	vkDestroyShaderModule(device, fragShaderModule, NULL);
	vkDestroyShaderModule(device, vertShaderModule, NULL);
	printf("Vulkan Graphics Pipeline Created!\n");
}

static void createFramebuffers(void) {
	swapchainFramebuffers = VECTOR_INIT(VkFramebuffer);
	vector_resize(&swapchainFramebuffers, swapchainImageViews.length);

	for (size_t i = 0; i < swapchainImageViews.length; i++) {
		VkImageView attachments[] = {
			VECTOR_GET(swapchainImageViews, VkImageView, i)};

		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = swapchainExtent.width,
			.height = swapchainExtent.height,
			.layers = 1,
		};

		VkFramebuffer fb = VK_NULL_HANDLE;
		if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &fb) !=
			VK_SUCCESS) {
			printf("Failed to create framebuffer %zu!\n", i);
			return;
		}
		VECTOR_GET(swapchainFramebuffers, VkFramebuffer, i) = fb;
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
	pickPhysicalDevice();
	if (physicalDevice == VK_NULL_HANDLE) {
		return;
	}
	createLogicalDevice();
	if (device == VK_NULL_HANDLE) {
		return;
	}

	createSwapchain(1920, 1080);
	if (swapchain == VK_NULL_HANDLE) {
		return;
	}

	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createSyncAndCommands();
}
void cleanupVulkan(void) {

	for (size_t i = 0; i < swapchainFramebuffers.length; i++) {
		vkDestroyFramebuffer(
			device, VECTOR_GET(swapchainFramebuffers, VkFramebuffer, i), NULL);
	}
	vector_free(&swapchainFramebuffers);

	if (graphicsPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(device, graphicsPipeline, NULL);
	if (pipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(device, pipelineLayout, NULL);
	if (renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(device, renderPass, NULL);

	if (device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(device);
		vkDestroyFence(device, inFlightFence, NULL);
		vkDestroySemaphore(device, renderFinishedSemaphore, NULL);
		vkDestroySemaphore(device, imageAvailableSemaphore, NULL);
		vkDestroyCommandPool(device, commandPool, NULL);
	}

	for (size_t i = 0; i < swapchainImageViews.length; i++) {
		VkImageView view = VECTOR_GET(swapchainImageViews, VkImageView, i);
		if (view != VK_NULL_HANDLE) {
			vkDestroyImageView(device, view, NULL);
		}
	}
	vector_free(&swapchainImageViews);

	vector_free(&swapchainImages);
	if (swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapchain, NULL);
		swapchain = VK_NULL_HANDLE;
		printf("Vulkan Swapchain Destroyed\n");
	}

	if (device != VK_NULL_HANDLE) {
		vkDestroyDevice(device, NULL);
		device = VK_NULL_HANDLE;
		printf("Vulkan Logical Device Destroyed\n");
	}
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
