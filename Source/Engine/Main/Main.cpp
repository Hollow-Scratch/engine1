#include "DynamicLibrary.hpp"

#include <stdio.h>
#include <stdlib.h>

typedef void (*GameInitFn)(void);
typedef void (*GameUpdateFn)(double delta_time);
typedef void (*GameShutdownFn)(void);

bool g_is_running = true;

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <game.so>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Load game module
	HollowEngine::DynamicLibrary game;

	if (!game.Load(argv[1])) {
		fprintf(stderr, "Failed to load game module: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	// Resolve game functions
	GameInitFn game_init = (GameInitFn)game.GetSymbol("HOLLOWENGINE_INIT");

	GameUpdateFn game_update =
		(GameUpdateFn)game.GetSymbol("HOLLOWENGINE_UPDATE");

	GameShutdownFn game_shutdown =
		(GameShutdownFn)game.GetSymbol("HOLLOWENGINE_SHUTDOWN");

	if (!game_init || !game_update || !game_shutdown) {
		fprintf(stderr, "Game module is missing required functions\n");
		game.UnLoad();
		return EXIT_FAILURE;
	}

	// Initialize game
	game_init();

	// Main loop
	while (g_is_running) {

		game_shutdown();

		// Unload game module
		game.UnLoad();

		return EXIT_SUCCESS;
	}
}
