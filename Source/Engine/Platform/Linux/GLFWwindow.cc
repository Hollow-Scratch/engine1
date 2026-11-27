#include "GLFWwindow.hpp"
#include "../../RuntimeError.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>

namespace HollowEngine {
namespace {
// GLFW initialization and window creation/destruction run on the main thread.
std::size_t windowCount = 0;

void checkGlfw(const char *operation, bool failed = false) {
	const char *description = nullptr;
	const int code = glfwGetError(&description);
	if (code != GLFW_NO_ERROR || failed) {
		throw std::runtime_error(std::format("{} (GLFW {:#x}): {}", operation, code,
											description ? description : "no error description"));
	}
}

void activateOpenGL(::GLFWwindow *window) {
	if (glfwGetCurrentContext() != window) {
		glfwMakeContextCurrent(window);
	}
	checkGlfw("activating OpenGL context");
}

void terminateGlfw() {
	glfwTerminate();
	checkGlfw("glfwTerminate");
}
} // namespace

GLFWwindow::~GLFWwindow() {
	RunGuarded("GLFW window cleanup", [this] { Shutdown(); });
}

void GLFWwindow::CreateWindow(const WindowsSpecification &specs) {
	if (!specs.Title || !specs.Width || !specs.Height ||
		!std::in_range<int>(specs.Width) || !std::in_range<int>(specs.Height)) {
		throw std::invalid_argument("invalid window title or dimensions");
	}
	if (!glfwInit()) {
		checkGlfw("glfwInit", true);
	}

	try {
		checkGlfw("glfwInit");
		const bool useOpenGL = specs.Api == GraphicsAPI::OpenGL;
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CLIENT_API, useOpenGL ? GLFW_OPENGL_API : GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, specs.isResizeable ? GLFW_TRUE : GLFW_FALSE);
		checkGlfw("configuring GLFW window hints");

		auto *monitor = specs.isFullScreen ? glfwGetPrimaryMonitor() : nullptr;
		if (specs.isFullScreen) {
			checkGlfw("glfwGetPrimaryMonitor", !monitor);
		}
		std::unique_ptr<::GLFWwindow, decltype(&glfwDestroyWindow)> candidate(
			glfwCreateWindow(static_cast<int>(specs.Width), static_cast<int>(specs.Height),
							 specs.Title, monitor, nullptr), glfwDestroyWindow);
		checkGlfw("glfwCreateWindow", !candidate);
		if (useOpenGL) {
			activateOpenGL(candidate.get());
			glfwSwapInterval(specs.Vsync ? 1 : 0);
			checkGlfw("glfwSwapInterval");
		}

		auto *previous = std::exchange(m_window, candidate.release());
		m_hasOpenGLContext = useOpenGL;
		if (previous) {
			glfwDestroyWindow(previous);
			checkGlfw("destroying the previous GLFW window");
		} else {
			++windowCount;
		}
	} catch (...) {
		if (windowCount == 0) {
			RunGuarded("GLFW initialization cleanup", terminateGlfw);
		}
		throw;
	}
}

void GLFWwindow::Update() {
	if (m_window) {
		if (m_hasOpenGLContext) {
			activateOpenGL(m_window);
		}
		glfwPollEvents();
		checkGlfw("glfwPollEvents");
	}
}

void GLFWwindow::SwapBuffers() {
	if (m_window && m_hasOpenGLContext) {
		activateOpenGL(m_window);
		glfwSwapBuffers(m_window);
		checkGlfw("glfwSwapBuffers");
	}
}

bool GLFWwindow::WindowShouldClose() const {
	if (!m_window) {
		return true;
	}
	const bool closing = glfwWindowShouldClose(m_window);
	checkGlfw("glfwWindowShouldClose");
	return closing;
}

void *GLFWwindow::GetWindowHandler() const { return m_window; }

void GLFWwindow::Shutdown() {
	if (!m_window) {
		return;
	}
	glfwDestroyWindow(std::exchange(m_window, nullptr));
	if (--windowCount == 0) {
		glfwTerminate();
	}
	checkGlfw("shutting down GLFW window");
}
} // namespace HollowEngine
