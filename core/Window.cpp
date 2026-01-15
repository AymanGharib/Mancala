#include "Window.h"
#include <glm/glm.hpp>           // ADD THIS
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>

Window::Window() : Window(Config{}) {}

Window::Window(const Config& config) : m_window(nullptr) {
    initGLFW(config);
    loadGLAD();
    setupCallbacks();

    // Configuration OpenGL initiale
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Anti-aliasing
    if (config.msaaSamples > 0) {
        glEnable(GL_MULTISAMPLE);
    }

    // Backface culling pour performances
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    setVSync(config.vsync);

    std::cout << "[Window] OpenGL " << glGetString(GL_VERSION) << std::endl;
    std::cout << "[Window] Renderer: " << glGetString(GL_RENDERER) << std::endl;
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Window::initGLFW(const Config& config) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.openglMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.openglMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    if (config.msaaSamples > 0) {
        glfwWindowHint(GLFW_SAMPLES, config.msaaSamples);
    }

    m_window = glfwCreateWindow(
        config.width, 
        config.height, 
        config.title.c_str(), 
        nullptr, 
        nullptr
    );

    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
}

void Window::loadGLAD() {
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

void Window::setupCallbacks() {
    // Framebuffer resize
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallbackStatic);
    
    // Mouse cursor position
    glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, double x, double y) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        self->onMouseMove(x, y);
    });

    // Mouse buttons
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, int button, int action, int mods) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        self->onMouseButton(button, action, mods);
    });

    // Mouse scroll
    glfwSetScrollCallback(m_window, [](GLFWwindow* w, double dx, double dy) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        self->onScroll(dx, dy);
    });

    // Keyboard
    glfwSetKeyCallback(m_window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        self->onKey(key, scancode, action, mods);
    });
}

void Window::framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height) {
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    glViewport(0, 0, width, height);
    
    if (win->m_framebufferCallback) {
        win->m_framebufferCallback(width, height);
    }
}

void Window::onMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_rightMouseDown = (action == GLFW_PRESS);
        if (m_rightMouseDown) {
            m_firstMouse = true; // Reset to avoid jump
        }
    }
}

void Window::onMouseMove(double x, double y) {
    if (m_firstMouse) {
        m_lastMouseX = x;
        m_lastMouseY = y;
        m_firstMouse = false;
    }

    double dx = x - m_lastMouseX;
    double dy = y - m_lastMouseY;
    m_lastMouseX = x;
    m_lastMouseY = y;

    // Right mouse button = orbit camera
    if (m_rightMouseDown) {
        const float sensitivity = 0.15f;
        m_yaw   += static_cast<float>(dx) * sensitivity;
        m_pitch -= static_cast<float>(dy) * sensitivity;
        
        // Clamp pitch to avoid gimbal lock
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
    }
}

void Window::onScroll(double dx, double dy) {
    // Zoom in/out by changing distance
    m_distance -= static_cast<float>(dy) * 0.5f;
    m_distance = glm::clamp(m_distance, 2.0f, 20.0f);
}

void Window::onKey(int key, int scancode, int action, int mods) {
    // ESC to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
    
    // Add more keyboard shortcuts here if needed
    // R = reset camera, F = toggle fullscreen, etc.
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::getFramebufferSize(int& width, int& height) const {
    glfwGetFramebufferSize(m_window, &width, &height);
}

void Window::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::setFramebufferSizeCallback(std::function<void(int, int)> callback) {
    m_framebufferCallback = std::move(callback);
}