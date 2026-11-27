#include "DynamicLibrary.hpp"
#include <limits.h>
#include <unistd.h>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>

std::filesystem::path GetExecutableDirectory() {
    char path[PATH_MAX];

    ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (length == -1)
        return {};

    path[length] = '\0';

    return std::filesystem::path(path).parent_path();
}
typedef void (*GameInitFn)(void);
typedef void (*GameUpdateFn)(double delta_time);
typedef void (*GameShutdownFn)(void);

bool g_is_running = true;

int main(void) {


	// Load game module
	auto game_path = GetExecutableDirectory() / "libgame.so";
	HollowEngine::DynamicLibrary game;

	if (!game.Load(game_path.c_str())) {
		fprintf(stderr, "Failed to load game module: %s\n", game_path.c_str());
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
		game_update(0.016);
	}

		game_shutdown();

		// Unload game module
		game.UnLoad();

		return EXIT_SUCCESS;
	
}
