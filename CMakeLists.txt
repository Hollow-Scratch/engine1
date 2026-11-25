cmake_minimum_required(VERSION 3.20)
project(
	HollowEngine
	VERSION 0.1.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable( HollowEngine
	Source/Engine/Main/Main.cpp
	Source/Engine/Main/DynamicLibrary.cpp
)

add_subdirectory(external/glfw)

find_package(Vulkan REQUIRED)

target_link_libraries(HollowEngine PRIVATE Vulkan::Vulkan glfw)

target_include_directories(HollowEngine PRIVATE
	"${PROJECT_SOURCE_DIR}/Source/Engine/"
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(HollowEngine PRIVATE -Wall -Wextra -Wstrict-prototypes)
endif()
