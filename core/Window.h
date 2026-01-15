#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class Window {
public:
    struct Config {
        int width = 1280;
        int height = 720;
        std::string title = "Mancala 3D";
        int openglMajor = 3;
        int openglMinor = 3;
        int msaaSamples = 4;
        bool vsync = true;
    };

    Window();
    explicit Window(const Config& config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    void getFramebufferSize(int& width, int& height) const;
    void setVSync(bool enabled);
    void setFramebufferSizeCallback(std::function<void(int, int)> callback);

    GLFWwindow* getGLFWWindow() const { return m_window; }

    // Camera control getters
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    float getDistance() const { return m_distance; }
    
    // Keyboard state
    bool isKeyPressed(int key) const { return glfwGetKey(m_window, key) == GLFW_PRESS; }

private:
    void initGLFW(const Config& config);
    void loadGLAD();
    void setupCallbacks();

    // Callback handlers
    static void framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height);
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double x, double y);
    void onScroll(double dx, double dy);
    void onKey(int key, int scancode, int action, int mods);

    GLFWwindow* m_window;
    std::function<void(int, int)> m_framebufferCallback;

    // Mouse state
    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_rightMouseDown = false;
    bool m_firstMouse = true;

    // Camera orbital controls
    float m_yaw = -90.0f;      // Horizontal rotation
    float m_pitch = -20.0f;    // Vertical rotation
    float m_distance = 8.0f;   // Distance from target

    // Camera panning (optional)
    float m_panX = 0.0f;
    float m_panY = 0.0f;
};