cmake_minimum_required(VERSION 3.20)
project(Snake_OpenGL LANGUAGES C CXX)

# set minimum C++ standart
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===========================
# executable
# ===========================
if (MSVC)
    add_executable(snake WIN32)
else()
    add_executable(snake)
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
)

# it doesn't work for clang
if (MSVC)
    target_link_options(snake PRIVATE
    /ENTRY:mainCRTStartup
    )
endif()

# ===========================
# GLAD
# ===========================

# glad.c as glad.lib
add_library(glad STATIC
    external/glad/src/glad.c
)

# includes glad.h to glad.c
target_include_directories(glad
    PUBLIC external/glad/include
)

# ===========================
# GLFW (haves CMakeLists.txt)
# ===========================
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

# just include its CMakeLists.txt
add_subdirectory(external/glfw)

# ===========================
# GLM (headers only)
# ===========================
target_include_directories(snake PRIVATE 
    external/glm
    external/stb_image
)

# ===========================
# Link libraries to exe
# ===========================
target_link_libraries(snake
    PRIVATE
        glad
        glfw
)