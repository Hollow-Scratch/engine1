#include "../Platform/Platform.hpp"
#include "DynamicLibrary.hpp"
#include <GL/gl.h>
#include <filesystem>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

int main() {
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
		return EXIT_FAILURE;
	}

	const HollowEngine::WindowsSpecification spec = {
		.Width = 1920, .Height = 1080, .Title = "HollowEngine",
		.Api = HollowEngine::GraphicsAPI::OpenGL};

	auto window = HollowEngine::Platform::Create();
	window->CreateWindow(spec);
	if (!window->GetWindowHandler()) {
		return EXIT_FAILURE;
	}

	game_init();
	while (!window->WindowShouldClose()) {
		window->Update();
		glClear(GL_COLOR_BUFFER_BIT);
		game_update(0.016);
		window->SwapBuffers();
	}
	game_shutdown();
	window->Shutdown();

	return EXIT_SUCCESS;
}
