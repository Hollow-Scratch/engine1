#ifndef HOLLOWENGINE_GLFWWINDOW_HEADER
#define HOLLOWENGINE_GLFWWINDOW_HEADER

#include "../Platform.hpp"

struct GLFWwindow;

namespace HollowEngine {
class GLFWwindow final : public Platform {
  public:
	GLFWwindow() = default;
	~GLFWwindow() override;
	GLFWwindow(const GLFWwindow &) = delete;
	GLFWwindow &operator=(const GLFWwindow &) = delete;

	void CreateWindow(const WindowsSpecification &specs) override;
	void Update() override;
	void SwapBuffers() override;
	void Shutdown() override;
	bool WindowShouldClose() const override;
	void *GetWindowHandler() const override;

  private:
	::GLFWwindow *m_window = nullptr;
	bool m_hasOpenGLContext = false;
};
} // namespace HollowEngine

#endif
