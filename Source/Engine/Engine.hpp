#pragma once

#include "Main/DynamicLibrary.hpp"
#include "Platform/Platform.hpp"

#include <memory>

namespace HollowEngine {

class Engine {
  public:
	int Run();

  private:
	void Initialize();
	bool RunLoop();

	DynamicLibrary m_game;
	std::unique_ptr<Platform> m_window;
	void (*m_gameInit)() = nullptr;
	void (*m_gameUpdate)(double) = nullptr;
	void (*m_gameShutdown)() = nullptr;
};

} // namespace HollowEngine
