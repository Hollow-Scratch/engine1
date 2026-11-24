#include "app.h"
#include "main/time.h"
#include "platform/platform.h"
#include "renderer/vulkan.h"

bool isRunning = true;

WindowSpecification specs = {
	.width	= 1920,
	.height = 1080,
	.title	= "HollowEngine",
};

int main(void) {
	initializeApp();

	platformInit(&specs);
	time_init();

	initializeVulkan();

	while (isRunning && !platformShouldClose()) {
		time_update();

		double dt = time_get_delta();

		updateApp(dt);
		drawVulkanFrame();

		platformUpdate();
	}

	cleanupVulkan();
	platformShutdown();
	shutdownApp();

	return 0;
}