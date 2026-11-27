#include "GLFWwindow.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstddef>
#include <iostream>
#include <utility>

namespace HollowEngine {
namespace {
// GLFW initialization and window creation/destruction run on the main thread.
std::size_t windowCount = 0;

void reportGlfwError(const char *operation) {
	const char *description = nullptr;
	glfwGetError(&description);
	std::cerr << operation << ": " << (description ? description : "unknown GLFW error") << '\n';
}

bool activateOpenGL(::GLFWwindow *window) {
	if (!window || glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_OPENGL_API) {
		return false;
	}
	if (glfwGetCurrentContext() != window) {
		glfwMakeContextCurrent(window);
		if (glfwGetCurrentContext() != window) {
			reportGlfwError("Failed to make the OpenGL context current");
			return false;
		}
	}
	return true;
}
} // namespace

GLFWwindow::~GLFWwindow() { Shutdown(); }

void GLFWwindow::CreateWindow(const WindowsSpecification &specs) {
	if (!specs.Title || !specs.Width || !specs.Height ||
		!std::in_range<int>(specs.Width) || !std::in_range<int>(specs.Height)) {
		std::cerr << "Invalid window title or dimensions\n";
		return;
	}
	if (!glfwInit()) {
		reportGlfwError("Failed to initialize GLFW");
		return;
	}

	const bool useOpenGL = specs.Api == GraphicsAPI::OpenGL;
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, useOpenGL ? GLFW_OPENGL_API : GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, specs.isResizeable ? GLFW_TRUE : GLFW_FALSE);
	auto *monitor = specs.isFullScreen ? glfwGetPrimaryMonitor() : nullptr;
	auto *window = glfwCreateWindow(static_cast<int>(specs.Width),
									static_cast<int>(specs.Height), specs.Title,
									monitor, nullptr);
	if (!window) {
		reportGlfwError("Failed to create GLFW window");
	} else if (useOpenGL) {
		if (activateOpenGL(window)) {
			glfwSwapInterval(specs.Vsync ? 1 : 0);
		} else {
			glfwDestroyWindow(window);
			window = nullptr;
		}
	}
	if (!window) {
		if (windowCount == 0) {
			glfwTerminate();
		}
		return;
	}

	if (m_window) {
		glfwDestroyWindow(m_window);
	} else {
		++windowCount;
	}
	m_window = window;
}

void GLFWwindow::Update() {
	if (m_window) {
		activateOpenGL(m_window);
		glfwPollEvents();
	}
}

void GLFWwindow::SwapBuffers() {
	if (activateOpenGL(m_window)) {
		glfwSwapBuffers(m_window);
	}
}

bool GLFWwindow::WindowShouldClose() const {
	return !m_window || glfwWindowShouldClose(m_window);
}

void *GLFWwindow::GetWindowHandler() const { return m_window; }

void GLFWwindow::Shutdown() {
	if (m_window) {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
		if (--windowCount == 0) {
			glfwTerminate();
		}
	}
}
} // namespace HollowEngine
