#include "../renderer/vulkan.h"
#include "app.h"
#include "platform.h"
#include <stdio.h>
#include <unistd.h>

bool Running = true;

WindowsSpecification specs = {
	.Height = 1920, .Width = 1080, .title = "HollowEngine"};

int main() {

	Init();
	platform_init(&specs);
	initVulkan();

	sleep(10);
	while (Running && !platform_shouldclose()) {
		Update(0.00000000000000000016);
		platform_update();
	}

	cleanupVulkan();
	platform_shutdown();
	Shutdown();

	return 0;
}
