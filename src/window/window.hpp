#include "src/core/action.hpp"
#include "src/core/vector.hpp"

#include <functional>

struct GLFWwindow;
struct GLFWcursor;

class Window {
  public:
    Window();
    ~Window();

    void setInputManagerSetKey(std::function<void(Action, bool, Mod)> fn) { inputManagerSetKey = std::move(fn); }
    void setRefreshCallback(std::function<void()> fn) { refreshScreen = std::move(fn); }
    
    auto getViewSize() const -> vec2i { return _viewSize; }

    void updateView(const vec2f fieldSize, const float contentScale, const bool zenMode);

    void close() const;
    auto shouldClose() -> bool;

    void endFrame();
    
  private:
    GLFWwindow* _handle = nullptr;
    GLFWcursor* _cursorHandle = nullptr;

    vec4i _windowParam;
    vec2i _bufferSize;
    vec2i _viewSize;
    float _windowScale;
    bool _fullscreen;

    std::function<void(Action key, bool isPressed, Mod mods)> inputManagerSetKey;
    std::function<void()> refreshScreen;

    void setViewport();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
    static void sizeCallback(GLFWwindow* window, int width, int height);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void contentSizeCallback(GLFWwindow* window, float xScale, float yScale);
    static void refreshCallback(GLFWwindow* window);
    static void errorCallback(const int error_code, const char *description);
};
