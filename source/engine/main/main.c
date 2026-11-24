#include "../renderer/vulkan.h"
#include "app.h"
#include "platform.h"

bool isRunning = true;

WindowSpecification specs = {
	.width = 1920,
	.height = 1080,
	.title = "HollowEngine",
};

int main(void) {
	initializeApp();
	platformInit(&specs);
	initializeVulkan();

	while (isRunning && !platformShouldClose()) {
		updateApp(0.016f);
		drawVulkanFrame();
		platformUpdate();
	}

	cleanupVulkan();
	platformShutdown();
	shutdownApp();

	return 0;
}
