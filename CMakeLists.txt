cmake_minimum_required(VERSION 3.20)
project(
	HollowEngine
	VERSION 0.1.0
	LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable( HollowEngine
	source/engine/main/main.c
	source/engine/main/game_test.c
	source/engine/main/platform.c
	source/engine/renderer/vulkan.c
	source/engine/utils/vector.c
)

add_subdirectory(external/glfw)

find_package(Vulkan REQUIRED)

target_link_libraries(HollowEngine PRIVATE Vulkan::Vulkan glfw)
