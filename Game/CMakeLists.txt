add_library(game SHARED
    game.cpp
)

target_include_directories(game PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(game PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME game
    LIBRARY_OUTPUT_DIRECTORY "${HOLLOW_OUTPUT_DIR}"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()
