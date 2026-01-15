#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // Constructor with all parameters
    Camera(const glm::vec3& position, const glm::vec3& target, 
           float fov, float aspect, float near, float far)
        : m_position(position)
        , m_target(target)
        , m_up(0.0f, 1.0f, 0.0f)
        , m_fov(fov)
        , m_aspect(aspect)
        , m_near(near)
        , m_far(far)
    {
        updateVectors();
    }

    // Simple constructor (orbital camera)
    Camera(const glm::vec3& target = glm::vec3(0.0f), float distance = 10.0f)
        : Camera(target + glm::vec3(0, 5, distance), target, 45.0f, 16.0f/9.0f, 0.1f, 100.0f)
    {}

    void setPosition(const glm::vec3& position) {
        m_position = position;
        updateVectors();
    }

    void lookAt(const glm::vec3& target) {
        m_target = target;
        updateVectors();
    }

    void setAspect(float aspect) {
        m_aspect = aspect;
    }

    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getTarget() const { return m_target; }
    glm::vec3 getFront() const { return m_front; }
    glm::vec3 getRight() const { return m_right; }
    glm::vec3 getUp() const { return m_up; }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(m_position, m_target, m_up);
    }

    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    }

private:
    void updateVectors() {
        m_front = glm::normalize(m_target - m_position);
        m_right = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;

    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;
};