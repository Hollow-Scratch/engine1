find_package(Vulkan REQUIRED)

add_executable(HollowEngine
    Main/Main.cpp
    Main/DynamicLibrary.cpp
)

target_include_directories(HollowEngine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Main"
)

target_link_libraries(HollowEngine PRIVATE
    Vulkan::Vulkan
    glfw
    ${CMAKE_DL_LIBS}
)

set_target_properties(HollowEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${HOLLOW_OUTPUT_DIR}"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(HollowEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
