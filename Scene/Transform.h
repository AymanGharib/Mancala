#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * @class Transform
 * @brief Représentation TRS (Translation-Rotation-Scale) d'un objet 3D
 * 
 * Utilise quaternions pour rotations (évite gimbal lock)
 * Cache la matrice Model pour optimisation
 */
class Transform {
public:
    Transform()
        : m_position(0.0f)
        , m_rotation(1.0f, 0.0f, 0.0f, 0.0f) // Quaternion identité
        , m_scale(1.0f)
        , m_modelMatrixDirty(true)
    {}

    // ===== SETTERS =====
    
    void setPosition(const glm::vec3& position) {
        m_position = position;
        m_modelMatrixDirty = true;
    }
    
    void setRotation(const glm::quat& rotation) {
        m_rotation = rotation;
        m_modelMatrixDirty = true;
    }
    
    /**
     * @brief Définit rotation depuis angles Euler (en degrés)
     */
    void setRotationEuler(float pitch, float yaw, float roll) {
        m_rotation = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
        m_modelMatrixDirty = true;
    }
    
    void setScale(const glm::vec3& scale) {
        m_scale = scale;
        m_modelMatrixDirty = true;
    }
    
    void setScale(float uniformScale) {
        setScale(glm::vec3(uniformScale));
    }

    // ===== GETTERS =====
    
    const glm::vec3& getPosition() const { return m_position; }
    const glm::quat& getRotation() const { return m_rotation; }
    const glm::vec3& getScale() const { return m_scale; }
    
    /**
     * @brief Retourne la matrice Model (avec cache)
     */
    glm::mat4 getModelMatrix() const {
        if (m_modelMatrixDirty) {
            updateModelMatrix();
        }
        return m_modelMatrix;
    }
    
    /**
     * @brief Direction "avant" de l'objet
     */
    glm::vec3 getForward() const {
        return m_rotation * glm::vec3(0, 0, -1);
    }
    
    glm::vec3 getRight() const {
        return m_rotation * glm::vec3(1, 0, 0);
    }
    
    glm::vec3 getUp() const {
        return m_rotation * glm::vec3(0, 1, 0);
    }

    // ===== TRANSFORMATIONS RELATIVES =====
    
    void translate(const glm::vec3& delta) {
        m_position += delta;
        m_modelMatrixDirty = true;
    }
    
    void rotate(float angleDegrees, const glm::vec3& axis) {
        glm::quat deltaRotation = glm::angleAxis(
            glm::radians(angleDegrees), 
            glm::normalize(axis)
        );
        m_rotation = deltaRotation * m_rotation;
        m_modelMatrixDirty = true;
    }
    
    void scale(const glm::vec3& factor) {
        m_scale *= factor;
        m_modelMatrixDirty = true;
    }
    
    /**
     * @brief Rotation "LookAt" - oriente l'objet vers une cible
     */
    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0,1,0)) {
        glm::vec3 direction = glm::normalize(target - m_position);
        glm::mat4 lookAtMatrix = glm::lookAt(m_position, target, worldUp);
        m_rotation = glm::quat_cast(glm::inverse(lookAtMatrix));
        m_modelMatrixDirty = true;
    }

private:
    /**
     * @brief Recalcule la matrice Model
     * Ordre : Scale → Rotate → Translate
     */
    void updateModelMatrix() const {
        m_modelMatrix = glm::mat4(1.0f);
        
        // Translation
        m_modelMatrix = glm::translate(m_modelMatrix, m_position);
        
        // Rotation (quaternion → matrice)
        m_modelMatrix *= glm::mat4_cast(m_rotation);
        
        // Scale
        m_modelMatrix = glm::scale(m_modelMatrix, m_scale);
        
        m_modelMatrixDirty = false;
    }

    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_scale;
    
    mutable glm::mat4 m_modelMatrix;
    mutable bool m_modelMatrixDirty;
};