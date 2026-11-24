#include "vulkan.h"

#include "../utils/vector.h"
#include "platform/platform.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkInstance		 g_instance		   = VK_NULL_HANDLE;
VkSurfaceKHR	 g_surface		   = VK_NULL_HANDLE;
VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;
VkDevice		 g_device		   = VK_NULL_HANDLE;
VkQueue			 g_graphics_queue  = VK_NULL_HANDLE;
VkQueue			 g_present_queue   = VK_NULL_HANDLE;
VkSwapchainKHR	 g_swapchain	   = VK_NULL_HANDLE;
VkFormat		 g_swapchain_image_format;
VkExtent2D		 g_swapchain_extent;
Vector			 g_swapchain_images			 = {0}; // Stores VkImage handles
Vector			 g_swapchain_image_views	 = {0}; // Stores VkImageView handles
VkCommandPool	 g_command_pool				 = VK_NULL_HANDLE;
VkCommandBuffer	 g_command_buffer			 = VK_NULL_HANDLE;
VkSemaphore		 g_image_available_semaphore = VK_NULL_HANDLE;
VkSemaphore		 g_render_finished_semaphore = VK_NULL_HANDLE;
VkFence			 g_in_flight_fence			 = VK_NULL_HANDLE;
VkRenderPass	 g_render_pass				 = VK_NULL_HANDLE;
VkPipelineLayout g_pipeline_layout			 = VK_NULL_HANDLE;
VkPipeline		 g_graphics_pipeline		 = VK_NULL_HANDLE;
Vector			 g_swapchain_framebuffers	 = {0}; // Stores VkFramebuffer handles

static VkDebugUtilsMessengerEXT g_debug_messenger = VK_NULL_HANDLE;

#ifdef NDEBUG
static const bool ENABLE_VALIDATION_LAYERS = false;
#else
static const bool ENABLE_VALIDATION_LAYERS = true;
#endif

static const char	 *VALIDATION_LAYERS[]	 = {"VK_LAYER_KHRONOS_validation"};
static const uint32_t VALIDATION_LAYER_COUNT = 1;

static const char	 *DEVICE_EXTENSIONS[]	 = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const uint32_t DEVICE_EXTENSION_COUNT = 1;

typedef struct {
	uint32_t graphics_family;
	uint32_t present_family;
	bool	 has_graphics;
	bool	 has_present;
} QueueFamilyIndices;

static bool IsQueueComplete(QueueFamilyIndices indices) {
	return indices.has_graphics && indices.has_present;
}

static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice target_device) {
	QueueFamilyIndices indices = {0};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(target_device, &queue_family_count, NULL);

	VECTOR_AUTO queue_families = VECTOR_INIT(VkQueueFamilyProperties);
	VectorResize(&queue_families, queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(target_device, &queue_family_count,
											 queue_families.data);

	for (uint32_t i = 0; i < queue_family_count; i++) {
		VkQueueFamilyProperties queue_family =
			VECTOR_GET(queue_families, VkQueueFamilyProperties, i);

		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
			indices.has_graphics	= true;
		}

		VkBool32 present_support = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(target_device, i, g_surface, &present_support);
		if (present_support) {
			indices.present_family = i;
			indices.has_present	   = true;
		}

		if (IsQueueComplete(indices)) {
			break;
		}
	}

	return indices;
}

static bool CheckDeviceExtensionSupport(VkPhysicalDevice target_device) {
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(target_device, NULL, &extension_count, NULL);

	VECTOR_AUTO available_extensions = VECTOR_INIT(VkExtensionProperties);
	VectorResize(&available_extensions, extension_count);
	vkEnumerateDeviceExtensionProperties(target_device, NULL, &extension_count,
										 available_extensions.data);

	for (uint32_t i = 0; i < DEVICE_EXTENSION_COUNT; i++) {
		bool found = false;
		for (size_t j = 0; j < available_extensions.length; j++) {
			VkExtensionProperties ext = VECTOR_GET(available_extensions, VkExtensionProperties, j);

			if (strcmp(DEVICE_EXTENSIONS[i], ext.extensionName) == 0) {
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

static bool IsDeviceSuitable(VkPhysicalDevice target_device) {
	QueueFamilyIndices indices				= FindQueueFamilies(target_device);
	bool			   extensions_supported = CheckDeviceExtensionSupport(target_device);

	bool swap_chain_adequate = false;
	if (extensions_supported) {
		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(target_device, g_surface, &format_count, NULL);

		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(target_device, g_surface, &present_mode_count,
												  NULL);

		swap_chain_adequate = (format_count > 0) && (present_mode_count > 0);
	}

	return IsQueueComplete(indices) && extensions_supported && swap_chain_adequate;
}

static void PickPhysicalDevice(void) {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(g_instance, &device_count, NULL);

	if (device_count == 0) {
		printf("Failed to find GPUs with Vulkan support!\n");
		return;
	}

	VECTOR_AUTO devices = VECTOR_INIT(VkPhysicalDevice);
	VectorResize(&devices, device_count);
	vkEnumeratePhysicalDevices(g_instance, &device_count, devices.data);

	printf("\nDetected Vulkan GPUs:\n");
	VkPhysicalDevice fallback_device = VK_NULL_HANDLE;

	for (size_t i = 0; i < devices.length; i++) {
		VkPhysicalDevice candidate = VECTOR_GET(devices, VkPhysicalDevice, i);

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(candidate, &props);
		printf("  [%zu] %s\n", i, props.deviceName);

		if (IsDeviceSuitable(candidate)) {
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				g_physical_device = candidate;
				break;
			}
			if (fallback_device == VK_NULL_HANDLE) {
				fallback_device = candidate;
			}
		}
	}

	if (g_physical_device == VK_NULL_HANDLE) {
		g_physical_device = fallback_device;
	}

	if (g_physical_device == VK_NULL_HANDLE) {
		printf("Failed to find a suitable GPU!\n");
		return;
	}

	VkPhysicalDeviceProperties selected_props;
	vkGetPhysicalDeviceProperties(g_physical_device, &selected_props);
	printf("Selected GPU: %s\n", selected_props.deviceName);
}

static void CreateLogicalDevice(void) {
	QueueFamilyIndices indices = FindQueueFamilies(g_physical_device);

	VECTOR_AUTO unique_queue_families = VECTOR_INIT(uint32_t);
	VectorAppend(&unique_queue_families, &indices.graphics_family);
	if (indices.present_family != indices.graphics_family) {
		VectorAppend(&unique_queue_families, &indices.present_family);
	}

	VECTOR_AUTO queue_create_infos = VECTOR_INIT(VkDeviceQueueCreateInfo);
	float		queue_priority	   = 1.0f;

	for (size_t i = 0; i < unique_queue_families.length; i++) {
		uint32_t family_index = VECTOR_GET(unique_queue_families, uint32_t, i);

		VkDeviceQueueCreateInfo queue_create_info = {
			.sType			  = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = family_index,
			.queueCount		  = 1,
			.pQueuePriorities = &queue_priority,
		};
		VectorAppend(&queue_create_infos, &queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features = {0};

	VkDeviceCreateInfo create_info = {
		.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount	 = (uint32_t)queue_create_infos.length,
		.pQueueCreateInfos		 = queue_create_infos.data,
		.pEnabledFeatures		 = &device_features,
		.enabledExtensionCount	 = DEVICE_EXTENSION_COUNT,
		.ppEnabledExtensionNames = DEVICE_EXTENSIONS,
	};

	if (vkCreateDevice(g_physical_device, &create_info, NULL, &g_device) != VK_SUCCESS) {
		printf("Failed to create logical device!\n");
		g_device = VK_NULL_HANDLE;
		return;
	}

	vkGetDeviceQueue(g_device, indices.graphics_family, 0, &g_graphics_queue);
	vkGetDeviceQueue(g_device, indices.present_family, 0, &g_present_queue);

	printf("Vulkan Logical Device & Queues Created Successfully!\n");
}

static uint32_t ClampU32(uint32_t value, uint32_t min, uint32_t max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

static void CreateSwapchain(uint32_t width, uint32_t height) {
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physical_device, g_surface, &capabilities);

	// 1. Pick Surface Format (prefer SRGB)
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_physical_device, g_surface, &format_count, NULL);
	VECTOR_AUTO formats = VECTOR_INIT(VkSurfaceFormatKHR);
	VectorResize(&formats, format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_physical_device, g_surface, &format_count, formats.data);

	VkSurfaceFormatKHR surface_format = VECTOR_GET(formats, VkSurfaceFormatKHR, 0);
	for (size_t i = 0; i < formats.length; i++) {
		VkSurfaceFormatKHR fmt = VECTOR_GET(formats, VkSurfaceFormatKHR, i);
		if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
			fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surface_format = fmt;
			break;
		}
	}

	// 2. Pick Present Mode (FIFO is guaranteed on all platforms including
	// Wayland)
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	// 3. Pick Swap Extent (Wayland sets current_extent to UINT32_MAX, so we
	// clamp our window size)
	VkExtent2D extent = capabilities.currentExtent;
	if (extent.width == UINT32_MAX) {
		extent.width =
			ClampU32(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = ClampU32(height, capabilities.minImageExtent.height,
								 capabilities.maxImageExtent.height);
	}

	uint32_t image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {
		.sType			  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface		  = g_surface,
		.minImageCount	  = image_count,
		.imageFormat	  = surface_format.format,
		.imageColorSpace  = surface_format.colorSpace,
		.imageExtent	  = extent,
		.imageArrayLayers = 1,
		.imageUsage		  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.preTransform	  = capabilities.currentTransform,
		.compositeAlpha	  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode	  = present_mode,
		.clipped		  = VK_TRUE,
		.oldSwapchain	  = VK_NULL_HANDLE,
	};

	QueueFamilyIndices indices				  = FindQueueFamilies(g_physical_device);
	uint32_t		   queue_family_indices[] = {indices.graphics_family, indices.present_family};

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode	  = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices	  = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	if (vkCreateSwapchainKHR(g_device, &create_info, NULL, &g_swapchain) != VK_SUCCESS) {
		printf("Failed to create swapchain!\n");
		g_swapchain = VK_NULL_HANDLE;
		return;
	}

	// Retrieve the swapchain VkImage handles into our long-lived Vector
	uint32_t actual_image_count = 0;
	vkGetSwapchainImagesKHR(g_device, g_swapchain, &actual_image_count, NULL);
	g_swapchain_images = VECTOR_INIT(VkImage);
	VectorResize(&g_swapchain_images, actual_image_count);
	vkGetSwapchainImagesKHR(g_device, g_swapchain, &actual_image_count, g_swapchain_images.data);

	g_swapchain_image_format = surface_format.format;
	g_swapchain_extent		 = extent;

	printf("Vulkan Swapchain Created (%ux%u, %u images)!\n", extent.width, extent.height,
		   actual_image_count);
}

static void CreateImageViews(void) {
	g_swapchain_image_views = VECTOR_INIT(VkImageView);
	VectorResize(&g_swapchain_image_views, g_swapchain_images.length);

	for (size_t i = 0; i < g_swapchain_images.length; i++) {
		VkImage image = VECTOR_GET(g_swapchain_images, VkImage, i);

		VkImageViewCreateInfo create_info = {
			.sType	  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image	  = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format	  = g_swapchain_image_format,
			.components =
				{
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY,
				},
			.subresourceRange =
				{
					.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel	= 0,
					.levelCount		= 1,
					.baseArrayLayer = 0,
					.layerCount		= 1,
				},
		};

		VkImageView image_view = VK_NULL_HANDLE;
		if (vkCreateImageView(g_device, &create_info, NULL, &image_view) != VK_SUCCESS) {
			printf("Failed to create image view %zu!\n", i);
			return;
		}
		VECTOR_GET(g_swapchain_image_views, VkImageView, i) = image_view;
	}

	printf("Vulkan Image Views Created (%zu views)!\n", g_swapchain_image_views.length);
}

static void CreateSyncAndCommands(void) {
	QueueFamilyIndices indices = FindQueueFamilies(g_physical_device);

	VkCommandPoolCreateInfo pool_info = {
		.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags			  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = indices.graphics_family,
	};
	vkCreateCommandPool(g_device, &pool_info, NULL, &g_command_pool);

	VkCommandBufferAllocateInfo alloc_info = {
		.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool		= g_command_pool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	vkAllocateCommandBuffers(g_device, &alloc_info, &g_command_buffer);

	VkSemaphoreCreateInfo sem_info	 = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo	  fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	vkCreateSemaphore(g_device, &sem_info, NULL, &g_image_available_semaphore);
	vkCreateSemaphore(g_device, &sem_info, NULL, &g_render_finished_semaphore);
	vkCreateFence(g_device, &fence_info, NULL, &g_in_flight_fence);
}

void DrawVulkanFrame(void) {
	vkWaitForFences(g_device, 1, &g_in_flight_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(g_device, 1, &g_in_flight_fence);

	uint32_t image_index = 0;
	vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, g_image_available_semaphore,
						  VK_NULL_HANDLE, &image_index);

	vkResetCommandBuffer(g_command_buffer, 0);
	VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	vkBeginCommandBuffer(g_command_buffer, &begin_info);

	VkClearValue		  clear_color	   = {{{0.05f, 0.05f, 0.08f, 1.0f}}};
	VkRenderPassBeginInfo render_pass_info = {
		.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass		 = g_render_pass,
		.framebuffer	 = VECTOR_GET(g_swapchain_framebuffers, VkFramebuffer, image_index),
		.renderArea		 = {.offset = {0, 0}, .extent = g_swapchain_extent},
		.clearValueCount = 1,
		.pClearValues	 = &clear_color,
	};

	vkCmdBeginRenderPass(g_command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(g_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphics_pipeline);
	vkCmdDraw(g_command_buffer, 3, 1, 0,
			  0); // Draws the 3 vertices of your triangle!
	vkCmdEndRenderPass(g_command_buffer);

	vkEndCommandBuffer(g_command_buffer);

	VkPipelineStageFlags wait_stage	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo		 submit_info = {
		.sType				  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount	  = 1,
		.pWaitSemaphores	  = &g_image_available_semaphore,
		.pWaitDstStageMask	  = &wait_stage,
		.commandBufferCount	  = 1,
		.pCommandBuffers	  = &g_command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores	  = &g_render_finished_semaphore,
	};
	vkQueueSubmit(g_graphics_queue, 1, &submit_info, g_in_flight_fence);

	VkPresentInfoKHR present_info = {
		.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores	= &g_render_finished_semaphore,
		.swapchainCount		= 1,
		.pSwapchains		= &g_swapchain,
		.pImageIndices		= &image_index,
	};
	vkQueuePresentKHR(g_present_queue, &present_info);
}

static void CreateRenderPass(void) {
	VkAttachmentDescription color_attachment = {
		.format			= g_swapchain_image_format,
		.samples		= VK_SAMPLE_COUNT_1_BIT,
		.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp		= VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout	= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint	  = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments	  = &color_attachment_ref,
	};

	VkSubpassDependency dependency = {
		.srcSubpass	   = VK_SUBPASS_EXTERNAL,
		.dstSubpass	   = 0,
		.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo render_pass_info = {
		.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments	 = &color_attachment,
		.subpassCount	 = 1,
		.pSubpasses		 = &subpass,
		.dependencyCount = 1,
		.pDependencies	 = &dependency,
	};

	if (vkCreateRenderPass(g_device, &render_pass_info, NULL, &g_render_pass) != VK_SUCCESS) {
		printf("Failed to create render pass!\n");
	}
}

static VkShaderModule CreateShaderModule(const char *filepath) {
	FILE *file = fopen(filepath, "rb");
	if (!file) {
		printf("Failed to open shader file: %s\n", filepath);
		return VK_NULL_HANDLE;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	rewind(file);

	char *buffer = malloc(file_size);
	fread(buffer, 1, file_size, file);
	fclose(file);

	VkShaderModuleCreateInfo create_info = {
		.sType	  = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = (size_t)file_size,
		.pCode	  = (const uint32_t *)buffer,
	};

	VkShaderModule shader_module = VK_NULL_HANDLE;
	if (vkCreateShaderModule(g_device, &create_info, NULL, &shader_module) != VK_SUCCESS) {
		printf("Failed to create shader module for %s!\n", filepath);
	}

	free(buffer);
	return shader_module;
}

static void CreateGraphicsPipeline(void) {
	VkShaderModule vert_shader_module = CreateShaderModule("shaders/vert.spv");
	VkShaderModule frag_shader_module = CreateShaderModule("shaders/frag.spv");
	if (vert_shader_module == VK_NULL_HANDLE || frag_shader_module == VK_NULL_HANDLE) {
		return;
	}

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		{
			.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage	= VK_SHADER_STAGE_VERTEX_BIT,
			.module = vert_shader_module,
			.pName	= "main",
		},
		{
			.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage	= VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = frag_shader_module,
			.pName	= "main",
		},
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport = {
		.x		  = 0.0f,
		.y		  = 0.0f,
		.width	  = (float)g_swapchain_extent.width,
		.height	  = (float)g_swapchain_extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = g_swapchain_extent,
	};

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType		   = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports	   = &viewport,
		.scissorCount  = 1,
		.pScissors	   = &scissor,
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType		 = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth	 = 1.0f,
		.cullMode	 = VK_CULL_MODE_BACK_BIT,
		.frontFace	 = VK_FRONT_FACE_CLOCKWISE,
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType				  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable	= VK_FALSE,
	};

	VkPipelineColorBlendStateCreateInfo color_blending = {
		.sType			 = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments	 = &color_blend_attachment,
	};

	VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	};
	vkCreatePipelineLayout(g_device, &pipeline_layout_info, NULL, &g_pipeline_layout);

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType				 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount			 = 2,
		.pStages			 = shader_stages,
		.pVertexInputState	 = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pViewportState		 = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState	 = &multisampling,
		.pColorBlendState	 = &color_blending,
		.layout				 = g_pipeline_layout,
		.renderPass			 = g_render_pass,
		.subpass			 = 0,
	};

	if (vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL,
								  &g_graphics_pipeline) != VK_SUCCESS) {
		printf("Failed to create graphics pipeline!\n");
	}

	vkDestroyShaderModule(g_device, frag_shader_module, NULL);
	vkDestroyShaderModule(g_device, vert_shader_module, NULL);
	printf("Vulkan Graphics Pipeline Created!\n");
}

static void CreateFramebuffers(void) {
	g_swapchain_framebuffers = VECTOR_INIT(VkFramebuffer);
	VectorResize(&g_swapchain_framebuffers, g_swapchain_image_views.length);

	for (size_t i = 0; i < g_swapchain_image_views.length; i++) {
		VkImageView attachments[] = {VECTOR_GET(g_swapchain_image_views, VkImageView, i)};

		VkFramebufferCreateInfo framebuffer_info = {
			.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass		 = g_render_pass,
			.attachmentCount = 1,
			.pAttachments	 = attachments,
			.width			 = g_swapchain_extent.width,
			.height			 = g_swapchain_extent.height,
			.layers			 = 1,
		};

		VkFramebuffer fb = VK_NULL_HANDLE;
		if (vkCreateFramebuffer(g_device, &framebuffer_info, NULL, &fb) != VK_SUCCESS) {
			printf("Failed to create framebuffer %zu!\n", i);
			return;
		}
		VECTOR_GET(g_swapchain_framebuffers, VkFramebuffer, i) = fb;
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT	  message_severity,
			  VkDebugUtilsMessageTypeFlagsEXT			  message_type,
			  const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data) {
	(void)message_severity;
	(void)message_type;
	(void)p_user_data;

	fprintf(stderr, "[Vulkan Validation]: %s\n", p_callback_data->pMessage);
	return VK_FALSE;
}

static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *create_info) {
	*create_info = (VkDebugUtilsMessengerCreateInfoEXT){
		.sType			 = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType	 = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
	};
}

static void SetupDebugMessenger(void) {
	if (!ENABLE_VALIDATION_LAYERS) {
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT create_info;
	PopulateDebugMessengerCreateInfo(&create_info);

	PFN_vkCreateDebugUtilsMessengerEXT func =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance,
																  "vkCreateDebugUtilsMessengerEXT");

	if (func != NULL) {
		func(g_instance, &create_info, NULL, &g_debug_messenger);
	}
}

static bool CheckValidationLayerSupport(void) {
	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	VECTOR_AUTO available_layers = VECTOR_INIT(VkLayerProperties);
	VectorResize(&available_layers, layer_count);

	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data);

	for (uint32_t i = 0; i < VALIDATION_LAYER_COUNT; i++) {
		bool found = false;

		for (size_t j = 0; j < available_layers.length; j++) {
			VkLayerProperties layer = VECTOR_GET(available_layers, VkLayerProperties, j);

			if (strcmp(VALIDATION_LAYERS[i], layer.layerName) == 0) {
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

static bool CheckExtensionSupport(const char **required_extensions, uint32_t required_count) {
	uint32_t extension_count = 0;

	VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

	if (result != VK_SUCCESS) {
		printf("Failed to enumerate Vulkan extensions: %d\n", result);
		return false;
	}

	VECTOR_AUTO available_extensions = VECTOR_INIT(VkExtensionProperties);
	VectorResize(&available_extensions, extension_count);

	if (available_extensions.data == NULL && extension_count > 0) {
		printf("Failed to allocate extension memory\n");
		return false;
	}

	result =
		vkEnumerateInstanceExtensionProperties(NULL, &extension_count, available_extensions.data);

	if (result != VK_SUCCESS) {
		printf("Failed to retrieve Vulkan extensions: %d\n", result);
		return false;
	}

	printf("\nAvailable Vulkan extensions:\n");

	for (size_t i = 0; i < available_extensions.length; i++) {
		VkExtensionProperties ext = VECTOR_GET(available_extensions, VkExtensionProperties, i);
		printf("  %s\n", ext.extensionName);
	}

	bool supported = true;

	for (uint32_t i = 0; i < required_count; i++) {
		bool found = false;

		for (size_t j = 0; j < available_extensions.length; j++) {
			VkExtensionProperties ext = VECTOR_GET(available_extensions, VkExtensionProperties, j);
			if (strcmp(required_extensions[i], ext.extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			printf("Missing required Vulkan extension: %s\n", required_extensions[i]);
			supported = false;
		}
	}
	return supported;
}
void CreateInstance(void) {
	if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport()) {
		printf("Validation layers requested, but not available!\n");
		return;
	}
	VkApplicationInfo app_info			  = {0};
	app_info.sType						  = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName			  = "HollowEngine";
	app_info.applicationVersion			  = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName				  = "HollowEngine";
	app_info.engineVersion				  = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion					  = VK_API_VERSION_1_0;
	uint32_t	 platform_extension_count = 0;
	const char **platform_extensions = PlatformGetRequiredExtensions(&platform_extension_count);
	if (platform_extensions == NULL || platform_extension_count == 0) {
		printf("Failed to get platform Vulkan extensions\n");
		return;
	}
	VECTOR_AUTO extensions = VECTOR_INIT(const char *);
	for (uint32_t i = 0; i < platform_extension_count; i++) {
		VectorAppend(&extensions, &platform_extensions[i]);
	}
	if (ENABLE_VALIDATION_LAYERS) {
		const char *debug_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		VectorAppend(&extensions, &debug_ext);
	}
	if (!CheckExtensionSupport(extensions.data, (uint32_t)extensions.length)) {
		printf("Required Vulkan extensions are unavailable\n");
		return;
	}
	VkInstanceCreateInfo create_info	= {0};
	create_info.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo		= &app_info;
	create_info.enabledExtensionCount	= (uint32_t)extensions.length;
	create_info.ppEnabledExtensionNames = extensions.data;
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
	if (ENABLE_VALIDATION_LAYERS) {
		create_info.enabledLayerCount	= VALIDATION_LAYER_COUNT;
		create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
		PopulateDebugMessengerCreateInfo(&debug_create_info);
		create_info.pNext = &debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext			  = NULL;
	}
	VkResult result = vkCreateInstance(&create_info, NULL, &g_instance);
	if (result != VK_SUCCESS) {
		printf("Failed to create Vulkan instance: %d\n", result);
		g_instance = VK_NULL_HANDLE;
		return;
	}
	SetupDebugMessenger();
	printf("\nVulkan Instance Created Successfully!\n");
}
void InitializeVulkan(void) {
	CreateInstance();
	if (g_instance == VK_NULL_HANDLE) {
		printf("Vulkan initialization failed\n");
		return;
	}
	if (!PlatformCreateVulkanSurface(g_instance, &g_surface)) {
		printf("Failed to create Vulkan window surface!\n");
		return;
	}
	printf("Vulkan Surface Created Successfully!\n");
	PickPhysicalDevice();
	if (g_physical_device == VK_NULL_HANDLE) {
		return;
	}
	CreateLogicalDevice();
	if (g_device == VK_NULL_HANDLE) {
		return;
	}

	CreateSwapchain(1920, 1080);
	if (g_swapchain == VK_NULL_HANDLE) {
		return;
	}

	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateSyncAndCommands();
}
void CleanupVulkan(void) {
	for (size_t i = 0; i < g_swapchain_framebuffers.length; i++) {
		vkDestroyFramebuffer(g_device, VECTOR_GET(g_swapchain_framebuffers, VkFramebuffer, i),
							 NULL);
	}
	VectorFree(&g_swapchain_framebuffers);

	if (g_graphics_pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(g_device, g_graphics_pipeline, NULL);
	if (g_pipeline_layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(g_device, g_pipeline_layout, NULL);
	if (g_render_pass != VK_NULL_HANDLE)
		vkDestroyRenderPass(g_device, g_render_pass, NULL);

	if (g_device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(g_device);
		vkDestroyFence(g_device, g_in_flight_fence, NULL);
		vkDestroySemaphore(g_device, g_render_finished_semaphore, NULL);
		vkDestroySemaphore(g_device, g_image_available_semaphore, NULL);
		vkDestroyCommandPool(g_device, g_command_pool, NULL);
	}

	for (size_t i = 0; i < g_swapchain_image_views.length; i++) {
		VkImageView view = VECTOR_GET(g_swapchain_image_views, VkImageView, i);
		if (view != VK_NULL_HANDLE) {
			vkDestroyImageView(g_device, view, NULL);
		}
	}
	VectorFree(&g_swapchain_image_views);

	VectorFree(&g_swapchain_images);
	if (g_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(g_device, g_swapchain, NULL);
		g_swapchain = VK_NULL_HANDLE;
		printf("Vulkan Swapchain Destroyed\n");
	}

	if (g_device != VK_NULL_HANDLE) {
		vkDestroyDevice(g_device, NULL);
		g_device = VK_NULL_HANDLE;
		printf("Vulkan Logical Device Destroyed\n");
	}
	if (g_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(g_instance, g_surface, NULL);
		g_surface = VK_NULL_HANDLE;
		printf("Vulkan Surface Destroyed\n");
	}
	if (ENABLE_VALIDATION_LAYERS && g_debug_messenger != VK_NULL_HANDLE) {
		PFN_vkDestroyDebugUtilsMessengerEXT func =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				g_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != NULL) {
			func(g_instance, g_debug_messenger, NULL);
			g_debug_messenger = VK_NULL_HANDLE;
		}
	}
	if (g_instance != VK_NULL_HANDLE) {
		vkDestroyInstance(g_instance, NULL);
		g_instance = VK_NULL_HANDLE;
		printf("Vulkan Instance Destroyed\n");
	}
}
