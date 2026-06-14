cmake_minimum_required(VERSION 3.20)
project(Snake_OpenGL LANGUAGES C CXX)

# set minimum C++ standart
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# for clangd extension
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DEBUG_MODE ON)

# ===========================
# executable
# ===========================

add_executable(snake)

# Set extra flags to strict compilation
if(MSVC)
    target_compile_options(snake PRIVATE /W4)
else()
    target_compile_options(snake PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set window or console mode
if(NOT DEBUG_MODE AND (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_link_options(snake PRIVATE
        -Xlinker /ENTRY:mainCRTStartup
    )
endif()

target_sources(snake PRIVATE
    src/main.cpp
    src/window/window.cpp
    src/render/renderer.cpp
    src/render/shader_program.cpp
    src/game/game.cpp
    src/game/field.cpp
    src/game/apple.cpp
    src/game/snake.cpp
    src/core/input.cpp
    src/core/config_manager.cpp
)

# ===========================
# GLAD
# ===========================

# glad.c as glad.lib
add_library(glad STATIC
    external/glad/src/glad.c
)

# includes glad.h to glad.c
target_include_directories(glad SYSTEM PUBLIC
    external/glad/include
)

# ===========================
# GLFW (haves CMakeLists.txt)
# ===========================
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

# just include its CMakeLists.txt
add_subdirectory(external/glfw SYSTEM)

# ===========================
# GLM (headers only)
# ===========================
target_include_directories(snake SYSTEM PRIVATE 
    external/glm
)

# ===========================
# Link libraries to exe
# ===========================
target_link_libraries(snake
    PRIVATE
        glad
        glfw
)