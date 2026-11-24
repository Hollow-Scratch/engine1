#include "../renderer/vulkan.h"
#include "app.h"
#include "platform.h"

bool Running = true;

WindowsSpecification specs = {
	.Width = 1920,
	.Height = 1080,
	.title = "HollowEngine",
};

int main(void) {
	Init();
	platform_init(&specs);
	initVulkan();

	while (Running && !platform_shouldclose()) {
		Update(0.016f);
		drawVulkanFrame();
		platform_update();
	}

	cleanupVulkan();
	platform_shutdown();
	Shutdown();

	return 0;
}
