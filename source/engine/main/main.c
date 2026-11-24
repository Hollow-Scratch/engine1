#include "main/dynamic_library.h"
#include "main/time.h"
#include "platform/platform.h"
#include "renderer/vulkan.h"

#include <stdio.h>
#include <stdlib.h>

typedef void (*GameInitFn)(void);
typedef void (*GameUpdateFn)(double delta_time);
typedef void (*GameShutdownFn)(void);

bool g_is_running = true;

WindowSpecification g_specs = {
	.width	= 1920,
	.height = 1080,
	.title	= "HollowEngine",
};

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <game.so>\n", argv[0]);
		return EXIT_FAILURE;
	}

	DynamicLibrary game = {0};

	if (!DynamicLibraryLoad(&game, argv[1])) {
		fprintf(stderr, "Failed to load game module\n");
		return EXIT_FAILURE;
	}

	GameInitFn game_init = (GameInitFn)DynamicLibraryGetSymbol(&game, "HOLLOWENGINE_INIT");

	GameUpdateFn game_update = (GameUpdateFn)DynamicLibraryGetSymbol(&game, "HOLLOWENGINE_UPDATE");

	GameShutdownFn game_shutdown =
		(GameShutdownFn)DynamicLibraryGetSymbol(&game, "HOLLOWENGINE_SHUTDOWN");

	if (!game_init || !game_update || !game_shutdown) {
		fprintf(stderr, "Game module is missing required functions\n");
		DynamicLibraryUnload(&game);
		return EXIT_FAILURE;
	}

	if (!PlatformInit(&g_specs)) {
		DynamicLibraryUnload(&game);
		return EXIT_FAILURE;
	}

	TimeInit();

	InitializeVulkan();

	game_init();

	while (g_is_running && !PlatformShouldClose()) {
		TimeUpdate();

		double dt = TimeGetDelta();

		game_update(dt);

		DrawVulkanFrame();

		PlatformUpdate();
	}

	game_shutdown();

	CleanupVulkan();
	PlatformShutdown();

	DynamicLibraryUnload(&game);

	return EXIT_SUCCESS;
}
