#ifndef HOLLOWENGINE_PLATFORM_HEADER
#define HOLLOWENGINE_PLATFORM_HEADER

#include <cstdint>
#include <memory>

namespace HollowEngine {

enum class GraphicsAPI { Vulkan, OpenGL };

struct WindowsSpecification {
	std::uint32_t Width{};
	std::uint32_t Height{};
	const char *Title{};
	bool Vsync = true;
	bool isResizeable = true;
	bool isFullScreen = false;
	GraphicsAPI Api = GraphicsAPI::Vulkan;
};

class Platform {
  public:
	virtual ~Platform() = default;

	static std::unique_ptr<Platform> Create();

	virtual void CreateWindow(const WindowsSpecification &specs) = 0;
	virtual void Update() = 0;
	// OpenGL presents here. Vulkan renderers present through their swapchain.
	virtual void SwapBuffers() = 0;
	virtual void Shutdown() = 0;
	virtual bool WindowShouldClose() const = 0;
	virtual void *GetWindowHandler() const = 0;
};
} // namespace HollowEngine

#endif
