#include "Platform.hpp"
#include "Linux/GLFWwindow.hpp"

namespace HollowEngine {
std::unique_ptr<Platform> Platform::Create() {
	return std::make_unique<GLFWwindow>();
}
} // namespace HollowEngine
