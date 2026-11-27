#include "Engine.hpp"
#include "RuntimeError.hpp"

#include <GL/gl.h>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <stdexcept>

namespace HollowEngine {
namespace {

template <typename Function>
Function requiredHook(DynamicLibrary &game, const char *name) {
	auto address = game.GetSymbol(name);
	if (!address) {
		throw std::runtime_error(std::format("missing game hook {}", name));
	}
	return reinterpret_cast<Function>(address);
}

void checkOpenGL() {
	const GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		throw std::runtime_error(std::format("OpenGL error {:#06x}", error));
	}
}

} // namespace

void Engine::Initialize() {
	const auto executable = std::filesystem::read_symlink("/proc/self/exe");
	const auto gamePath = executable.parent_path() / "libgame.so";
	if (!m_game.Load(gamePath.c_str())) {
		throw std::runtime_error(std::format("could not load {}", gamePath.string()));
	}
	m_gameInit = requiredHook<decltype(m_gameInit)>(m_game, "HOLLOWENGINE_INIT");
	m_gameUpdate = requiredHook<decltype(m_gameUpdate)>(m_game, "HOLLOWENGINE_UPDATE");
	m_gameShutdown = requiredHook<decltype(m_gameShutdown)>(m_game, "HOLLOWENGINE_SHUTDOWN");

	m_window = Platform::Create();
	if (!m_window) {
		throw std::runtime_error("Platform::Create returned no platform");
	}
	m_window->CreateWindow({.Width = 1920, .Height = 1080, .Title = "HollowEngine",
							.Api = GraphicsAPI::OpenGL});
	if (!m_window->GetWindowHandler()) {
		throw std::runtime_error("Platform::CreateWindow returned no window");
	}
}

bool Engine::RunLoop() {
	const char *operation = "Engine update/render loop";
	try {
		for (;;) {
			operation = "Platform event polling";
			m_window->Update();
			if (m_window->WindowShouldClose()) {
				return true;
			}

			operation = "OpenGL frame clear";
			glClear(GL_COLOR_BUFFER_BIT);
			checkOpenGL();

			operation = "HOLLOWENGINE_UPDATE";
			m_gameUpdate(0.016);
			checkOpenGL();

			operation = "Platform buffer swap";
			m_window->SwapBuffers();
		}
	} catch (const std::exception &error) {
		std::fprintf(stderr, "%s failed: %s\n", operation, error.what());
	} catch (...) {
		std::fprintf(stderr, "%s failed: unknown exception\n", operation);
	}
	return false;
}

int Engine::Run() {
	bool ok = RunGuarded("Engine initialization", [&] { Initialize(); }) &&
			  RunGuarded("HOLLOWENGINE_INIT", m_gameInit);
	if (ok) {
		ok = RunGuarded("HOLLOWENGINE_INIT graphics", checkOpenGL) && RunLoop();
		if (!RunGuarded("HOLLOWENGINE_SHUTDOWN", [this] {
				m_gameShutdown();
				checkOpenGL();
			})) {
			ok = false;
		}
	}
	if (m_window && !RunGuarded("Platform shutdown", [&] { m_window->Shutdown(); })) {
		ok = false;
	}
	m_window.reset();
	if (!RunGuarded("Game module unload", [&] { m_game.UnLoad(); })) {
		ok = false;
	}
	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace HollowEngine
