#include "window.hpp" 

#include "../render/textures/apple_texture.hpp"
#include "../core/action.hpp"
#include "../config/window_config.hpp"
#include "../core/logger.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h> // after glad

#include <string>

inline static Action keyMap[GLFW_KEY_LAST + 1];

Window::Window() :
      terminated(false),
      updateTitle(false),
      scoreTitle(-1),
      fpsTitle(0)
    {
    if (!glfwInit()) {
        Logger::getInstance().printError("Window", "GLFW_initialization_failed");
        exit(1);
    }

    // Set OpenGL version 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Set window invisible to setting it without visible effects
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Set monitor info
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    m_mWidth = mode->width;
    m_mHeight = mode->height;
    m_mRefreshRate = mode->refreshRate;
    // Set window info
    m_prevWidth = m_width = WindowConfigVariables::windowWidth;
    m_prevHeight = m_height = WindowConfigVariables::windowHeight;
    m_fullscreen = WindowConfigVariables::fullscreen;
    m_windowXPos = m_mWidth / 2 - m_width / 2;
    m_windowYPos = m_mHeight / 2 - m_height / 2;

    // Create window
    m_handle = glfwCreateWindow(
        m_width,
        m_height,
        "Snake | Score: 0 | FPS: ",
        nullptr,
        nullptr
    );
    if (!m_handle) {
        Logger::getInstance().printError("Window", "GLFW_window_creation_failed");
        terminate();
        exit(1);
    }

    glfwMakeContextCurrent(m_handle);

    // set pointer to this window -- it's needed to work with some callback funcs
    glfwSetWindowUserPointer(m_handle, this);    
    glfwSetWindowSizeLimits(m_handle, 800, 800, m_mWidth, m_mHeight);
    glfwSetWindowPos(m_handle, m_windowXPos, m_windowYPos);

    // Load OpenGL funcs
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::getInstance().printError("Window", "GLFW_load_OpenGL_funcs_failed");
        terminate();
        exit(1);
    }

    // Set window icon title
    GLFWimage icon;
    icon.width = 48;
    icon.height = 48;
    icon.pixels = applePixels;
    glfwSetWindowIcon(m_handle, 1, &icon);

    if (m_fullscreen) {
        glfwSetWindowMonitor(
            m_handle,
            glfwGetPrimaryMonitor(),
            0,
            0,
            m_mWidth,
            m_mHeight,
            GLFW_DONT_CARE
        );
        glViewport((m_mWidth - 800) / 2, (m_mHeight - 800) / 2, 800, 800);
    } else {
        glViewport((m_width - 800) / 2, (m_height - 800) / 2, 800, 800);
    }

    // Callback funcs
    glfwSetWindowSizeCallback(m_handle, &Window::sizeCallback);
    glfwSetErrorCallback(&Window::errorCallback);
    glfwSetKeyCallback(m_handle, &Window::keyCallback);
    glfwSetWindowRefreshCallback(m_handle, &Window::refreshCallback);

    glfwSwapInterval(1);
    glfwShowWindow(m_handle);

    mapKey();
}

void Window::close() const { glfwSetWindowShouldClose(m_handle, GLFW_TRUE); }
bool Window::shouldClose() { return static_cast<bool>(glfwWindowShouldClose(m_handle)); }

void Window::mapKey() {
    for (int i = 0; i <= GLFW_KEY_LAST; ++i)
        keyMap[i] = Action::COUNT;
    
    keyMap[GLFW_KEY_ESCAPE] = Action::Exit;
    keyMap[GLFW_KEY_P] = Action::Pause;

    keyMap[GLFW_KEY_W]  = Action::MoveUp;
    keyMap[GLFW_KEY_UP] = Action::MoveUp;

    keyMap[GLFW_KEY_A]    = Action::MoveLeft;
    keyMap[GLFW_KEY_LEFT] = Action::MoveLeft;

    keyMap[GLFW_KEY_S]    = Action::MoveDown;
    keyMap[GLFW_KEY_DOWN] = Action::MoveDown;

    keyMap[GLFW_KEY_D]     = Action::MoveRight;
    keyMap[GLFW_KEY_RIGHT] = Action::MoveRight;
}

void Window::setTitle() {
    std::string title = "Snake | Score: " + std::to_string(scoreTitle) + " | FPS: " + std::to_string(fpsTitle);
    glfwSetWindowTitle(m_handle, title.c_str());
}

void Window::updateScore(const bool increment) {
    scoreTitle = increment ? scoreTitle + 1 : -1;
    updateTitle = true;
}

void Window::pollEvents() {
    glfwPollEvents();

    if (updateTitle) {
        setTitle();
        updateTitle = false;
    }
}

void Window::swapBuffers() { glfwSwapBuffers(m_handle); }

void Window::terminate() { 
    if (!terminated) {
        glfwTerminate(); 
        terminated = true;
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (mods == GLFW_MOD_ALT && action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
        if (self) {
            if (!self->m_fullscreen) {
                self->m_prevWidth = self->m_width;
                self->m_prevHeight = self->m_height;
                glfwSetWindowMonitor(
                    window, 
                    glfwGetPrimaryMonitor(), 
                    0, 
                    0, 
                    self->m_mWidth, 
                    self->m_mHeight, 
                    GLFW_DONT_CARE
                );
            } else {
                glfwSetWindowMonitor(
                    window,
                    nullptr,
                    self->m_windowXPos,
                    self->m_windowYPos,
                    self->m_prevWidth,
                    self->m_prevHeight,
                    GLFW_DONT_CARE
                );
            }

            self->m_fullscreen = !self->m_fullscreen;
        }
    }   

    Action aKey = keyMap[key];
    if (aKey == Action::COUNT)
        return;
    
    self->inputManagerSetKey(aKey, action);
}

void Window::errorCallback(const int error_code, const char *description) {
    std::string strDescription = "(" + std::to_string(error_code) + ") " + description;
    Logger::getInstance().printError("WINDOW", strDescription.c_str());
}

void Window::sizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (self) {
        self->m_width = width;
        self->m_height = height;
    }
 
    glViewport((width - 800) / 2, (height - 800) / 2, 800, 800); 
}

void Window::refreshCallback(GLFWwindow* window) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (self->refreshScreen)
        self->refreshScreen();

    glfwSwapBuffers(window);
}