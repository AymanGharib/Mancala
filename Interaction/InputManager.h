#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

/**
 * @class InputManager
 * @brief Gestion centralisée des inputs clavier/souris
 */
class InputManager {
public:
    static InputManager& getInstance() {
        static InputManager instance;
        return instance;
    }

    void init(GLFWwindow* window) {
        m_window = window;
        
        // Callbacks GLFW
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
        
        // Position initiale souris
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        m_mousePos = glm::vec2(x, y);
        m_lastMousePos = m_mousePos;
    }

    void update() {
        m_mouseDelta = m_mousePos - m_lastMousePos;
        m_lastMousePos = m_mousePos;
        m_scrollDelta = 0.0f; // Reset chaque frame
    }

    bool isMouseButtonPressed(int button) const {
        return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
    }

    bool isKeyPressed(int key) const {
        return glfwGetKey(m_window, key) == GLFW_PRESS;
    }

    glm::vec2 getMousePosition() const { return m_mousePos; }
    glm::vec2 getMouseDelta() const { return m_mouseDelta; }
    float getScrollDelta() const { return m_scrollDelta; }

private:
    InputManager() = default;

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        // Peut être étendu pour stocker état boutons
    }

    static void cursorPosCallback(GLFWwindow* window, double x, double y) {
        InputManager::getInstance().m_mousePos = glm::vec2(x, y);
    }

    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
        InputManager::getInstance().m_scrollDelta = static_cast<float>(yOffset);
    }

    GLFWwindow* m_window = nullptr;
    glm::vec2 m_mousePos{0.0f};
    glm::vec2 m_lastMousePos{0.0f};
    glm::vec2 m_mouseDelta{0.0f};
    float m_scrollDelta = 0.0f;
};