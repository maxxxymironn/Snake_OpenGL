class GLFWwindow;

class Window {
    GLFWwindow* m_handle = nullptr;

    // Monitor info
    int m_mWidth;
    int m_mHeight;
    int m_mRefreshRate;

    // Window info
    int m_width;
    int m_height;
    int m_prevWidth;
    int m_prevHeight;

    int m_windowXPos;
    int m_windowYPos;
    bool m_fullscreen;

    bool terminated;

    // Title info
    bool updateTitle;
    short scoreTitle;
    int fpsTitle;

    void mapKey();

    void setTitle();

    static void errorCallback(const int error_code, const char *description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void sizeCallback(GLFWwindow* window, int width, int height);
    static void refreshCallback(GLFWwindow* window) {};

public:
    Window();
    ~Window() { terminate(); }
    
    void terminate();

    void close() const;
    bool shouldClose();
    
    void pollEvents();
    void swapBuffers();

    void updateFPS(const int fps) { fpsTitle = fps; updateTitle = true; }
    void updateScore(const bool increment=true);
};