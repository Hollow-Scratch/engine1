find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(HollowEngine
				main.cc
				Engine.cc
				Main/DynamicLibrary.cc
				Platform/Platform.cc
				Platform/Linux/GLFWwindow.cc
)

target_include_directories(HollowEngine PRIVATE
				"${CMAKE_CURRENT_SOURCE_DIR}/Main"
)

target_link_libraries(HollowEngine PRIVATE
				Vulkan::Vulkan
				OpenGL::GL
				glfw
				${CMAKE_DL_LIBS}
)

set_target_properties(HollowEngine PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY "${HOLLOW_OUTPUT_DIR}"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(HollowEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
