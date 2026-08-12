#include "../core/action.hpp"

#include "../core/vector.hpp"

#include <functional>

class GLFWwindow;

class Window {
    GLFWwindow* _handle = nullptr;

    vec4i _windowParam;
    vec2i _bufferSize;
    vec2i _viewSize;
    vec2i _fieldSize;

    float _windowScale;
    float _contentScale;
    int _scoreTitle;
    int _fpsTitle;

    bool _fullscreen;
    bool _zenMode;
    bool _updateTitle;

    std::function<void(Action key, bool isPressed, Mod mods)> inputManagerSetKey;
    std::function<void()> refreshScreen;

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void sizeCallback(GLFWwindow* window, int width, int height);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void contentSizeCallback(GLFWwindow* window, float xScale, float yScale);
    static void refreshCallback(GLFWwindow* window);
    static void errorCallback(const int error_code, const char *description);

    void setTitle();
    void setViewport();
    void updateView();
public:
    Window();
    ~Window();

    void setInputManagerSetKey(std::function<void(Action, bool, Mod)> fn) { inputManagerSetKey = std::move(fn); }
    void setRefreshCallback(std::function<void()> fn) { refreshScreen = std::move(fn); }

    vec2i getViewSize() const { return _viewSize; }
    float getContentScale() const { return _contentScale; }

    void updateFieldSize(const vec2i fieldCellSize) { _fieldSize = fieldCellSize * 40; updateView(); }
    void increaseContentScaling() { _contentScale += 0.1f; updateView(); }
    void decreaseContentScaling() { _contentScale -= 0.1f; updateView(); }
    void changeZenStatus() { _zenMode = !_zenMode; updateView(); }

    void close() const;
    bool shouldClose();
        
    void pollEvents();

    void swapBuffers();

    void updateFPS(const int fps) { _fpsTitle = fps; _updateTitle = true; }
    void updateScore(const bool toIncrement=true);
};