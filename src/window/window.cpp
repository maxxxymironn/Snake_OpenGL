#include "window.hpp"

#include "src/config/window_config.hpp"
#include "src/core/action.hpp"
#include "src/core/logger.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
// #include <iostream>

namespace {
    Action keyMap[GLFW_KEY_LAST + 1];

    void mapKey() {
        for (int i = 0; i <= GLFW_KEY_LAST; ++i)
            keyMap[i] = Action::COUNT;

        keyMap[GLFW_KEY_ESCAPE] = Action::EXIT;
        keyMap[GLFW_KEY_P] = Action::PAUSE;
        keyMap[GLFW_KEY_SPACE] = Action::SPACE;
        keyMap[GLFW_KEY_Y] = Action::YES;
        keyMap[GLFW_KEY_N] = Action::NO;

        keyMap[GLFW_KEY_W]  = Action::MOVE_UP;
        keyMap[GLFW_KEY_UP] = Action::MOVE_UP;
        keyMap[GLFW_KEY_A]    = Action::MOVE_LEFT;
        keyMap[GLFW_KEY_LEFT] = Action::MOVE_LEFT;
        keyMap[GLFW_KEY_S]    = Action::MOVE_DOWN;
        keyMap[GLFW_KEY_DOWN] = Action::MOVE_DOWN;
        keyMap[GLFW_KEY_D]     = Action::MOVE_RIGHT;
        keyMap[GLFW_KEY_RIGHT] = Action::MOVE_RIGHT;

        keyMap[GLFW_KEY_EQUAL] = Action::SCALE_UP;
        keyMap[GLFW_KEY_MINUS] = Action::SCALE_DOWN;
        keyMap[GLFW_KEY_Z] = Action::ZEN_MODE;
        keyMap[GLFW_KEY_M] = Action::PRAVEDNOV;
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (self) {
        Mod selfMods = Mod::EMPTY;
        if (mods == GLFW_MOD_CONTROL)
            selfMods = Mod::CTRL;

        // fullscreen -- ALT ENTER
        if (mods == GLFW_MOD_ALT && action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            if (!self->_fullscreen) {
                glfwGetWindowPos(self->_handle, &self->_windowParam.z, &self->_windowParam.w);
                glfwSetWindowMonitor(
                    window, glfwGetPrimaryMonitor(),
                    0, 0, mode->width, mode->height,
                    mode->refreshRate
                );
            } else {
                glfwSetWindowMonitor(
                    window, nullptr,
                    self->_windowParam.z, self->_windowParam.w,
                    self->_windowParam.x, self->_windowParam.y,
                    mode->refreshRate
                );
                // self->setViewport();
            }
            self->_fullscreen = !self->_fullscreen;
        }

        if (keyMap[key] == Action::COUNT)
            return;

        self->inputManagerSetKey(keyMap[key], action, selfMods);
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        // send left button information to inputHandler
    }
}

void Window::cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        // send new CursorPos to InputHandler
    }
}

void Window::sizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (self && width != mode->width && height != mode->height) {
        self->_windowParam.x = width;
        self->_windowParam.y = height;
    }
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->_bufferSize = {width, height};
        self->setViewport();
    }
}

void Window::contentSizeCallback(GLFWwindow* window, float xScale, float yScale) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->_windowScale = xScale;

        if (xScale != yScale)
            Logger::getInstance().printInfo("WINDOW", "xScale != yScale. xScale will be used");
    }
}

void Window::refreshCallback(GLFWwindow* window) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->refreshScreen)
        self->refreshScreen();

    glfwSwapBuffers(window);
}

void Window::errorCallback(const int error_code, const char *description) {
    std::string strDescription = "(" + std::to_string(error_code) + ") " + description;
    Logger::getInstance().printError("WINDOW", strDescription.c_str());
}

void Window::setViewport() {
    vec2i pos = (_bufferSize - static_cast<vec2i>(static_cast<vec2f>(_viewSize) * _windowScale )) / 2;
    vec2i size = static_cast<vec2i>(static_cast<vec2f>(_viewSize) * _windowScale);
    glViewport(pos.x, pos.y, size.x, size.y);
}

void Window::updateView(const vec2f rawViewSize, const float contentScale, const bool zenMode) {
    if (zenMode) {
        _viewSize = rawViewSize * contentScale;
        _windowParam.x = _viewSize.x;
        _windowParam.y = _viewSize.y;
    }
    else {
        _viewSize = static_cast<vec2f>(rawViewSize) * contentScale;

        if (_windowParam.x < _viewSize.x)
            _windowParam.x = _viewSize.x;
        if (_windowParam.y < _viewSize.y)
            _windowParam.y = _viewSize.y;
    }

    glfwSetWindowSizeLimits(
        _handle,
        _viewSize.x, _viewSize.y,
        GLFW_DONT_CARE, GLFW_DONT_CARE
    );

    glfwSetWindowSize(_handle, _windowParam.x, _windowParam.y);

    setViewport();
}

Window::Window()
    : _windowParam(WindowConfig::windowWidth, WindowConfig::windowHeight, 0, 0)
    , _bufferSize(WindowConfig::windowWidth, WindowConfig::windowHeight)
    , _viewSize(0, 0)
    , _windowScale(1.f)
    , _fullscreen(WindowConfig::fullscreen)
{
    if (!glfwInit()) {
        Logger::getInstance().printError("Window", "GLFW initialization failed");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    _windowParam.z = mode->width / 2 - _windowParam.x / 2;
    _windowParam.w = mode->height / 2 - _windowParam.y / 2;

    _handle = glfwCreateWindow(
        _windowParam.x, _windowParam.y,
        "Snake",
        nullptr, nullptr
    );
    if (!_handle) {
        Logger::getInstance().printError("Window", "window creation failed");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(_handle);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::getInstance().printError("Window", "OpenGL funcs initialization failed");
        glfwTerminate();
        exit(1);
    }

    glfwSetWindowUserPointer(_handle, this);
    glfwSetWindowSizeLimits(_handle, 1200, 800, mode->width, mode->height);
    glfwSetWindowPos(_handle, _windowParam.z, _windowParam.w);

    _cursorHandle = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    // GLFWimage icon;
    // icon.width = 48;
    // icon.height = 48;
    // icon.pixels = applePixels;
    // glfwSetWindowIcon(m_handle, 1, &icon);

    mapKey();

    glfwSetWindowSizeCallback(_handle, Window::sizeCallback);
    glfwSetFramebufferSizeCallback(_handle, Window::framebufferSizeCallback);
    glfwSetWindowRefreshCallback(_handle, Window::refreshCallback);
    glfwSetKeyCallback(_handle, Window::keyCallback);
    glfwSetMouseButtonCallback(_handle, Window::mouseButtonCallback);
    glfwSetCursorPosCallback(_handle, Window::cursorPosCallback);
    glfwSetWindowContentScaleCallback(_handle, Window::contentSizeCallback);
    glfwSetErrorCallback(Window::errorCallback);

    glfwSwapInterval(GLFW_TRUE);
    glfwShowWindow(_handle);

    if (_fullscreen) {
        glfwSetWindowMonitor(
            _handle, glfwGetPrimaryMonitor(),
            0, 0, mode->width, mode->height,
            mode->refreshRate
        );
    }

    setViewport();
}

Window::~Window() {
    glfwDestroyCursor(_cursorHandle);
    glfwTerminate();

    WindowConfig::fullscreen = _fullscreen;
    WindowConfig::windowWidth = _windowParam.x;
    WindowConfig::windowHeight = _windowParam.y;
}

void Window::close() const { glfwSetWindowShouldClose(_handle, GLFW_TRUE); }
bool Window::shouldClose() { return static_cast<bool>(glfwWindowShouldClose(_handle)); }

void Window::endFrame() {
    glfwSwapBuffers(_handle);
    glfwPollEvents();
}
