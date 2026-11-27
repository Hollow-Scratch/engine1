cmake_minimum_required(VERSION 3.20)

project(HollowEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HOLLOW_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(external/glfw)
add_subdirectory(Source/Engine)
add_subdirectory(Game)
