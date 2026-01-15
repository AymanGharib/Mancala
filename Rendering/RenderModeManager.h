#pragma once

#include <glad/gl.h>
#include <string>

/**
 * @brief Gestion des modes d'affichage (wireframe, shaded, textured)
 * Requis par le cahier des charges du projet
 */
class RenderModeManager {
public:
    enum class Mode {
        WIREFRAME,      // Fil de fer uniquement
        SHADED,         // Surfaces pleines sans textures
        TEXTURED,       // Surfaces avec textures
        SHADED_WIRE     // Surfaces + fil de fer (bonus)
    };

    static RenderModeManager& getInstance() {
        static RenderModeManager instance;
        return instance;
    }

    /**
     * @brief Change le mode de rendu
     */
    void setMode(Mode mode) {
        m_currentMode = mode;
        applyMode();
    }

    /**
     * @brief Cycle à travers les modes (pour touche clavier)
     */
    void cycleMode() {
        int modeInt = static_cast<int>(m_currentMode);
        modeInt = (modeInt + 1) % 4;  // 4 modes au total
        m_currentMode = static_cast<Mode>(modeInt);
        applyMode();
    }

    Mode getCurrentMode() const { return m_currentMode; }
    
    std::string getModeName() const {
        switch (m_currentMode) {
            case Mode::WIREFRAME:   return "Wireframe";
            case Mode::SHADED:      return "Shaded";
            case Mode::TEXTURED:    return "Textured";
            case Mode::SHADED_WIRE: return "Shaded + Wireframe";
            default:                return "Unknown";
        }
    }

    /**
     * @brief Vérifie si les textures doivent être utilisées
     */
    bool shouldUseTextures() const {
        return m_currentMode == Mode::TEXTURED || m_currentMode == Mode::SHADED_WIRE;
    }

    /**
     * @brief Vérifie si le wireframe doit être dessiné
     */
    bool shouldDrawWireframe() const {
        return m_currentMode == Mode::WIREFRAME || m_currentMode == Mode::SHADED_WIRE;
    }

    /**
     * @brief Configure OpenGL pour le mode actuel
     */
    void applyMode() {
        switch (m_currentMode) {
            case Mode::WIREFRAME:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_CULL_FACE);  // Voir toutes les faces en wireframe
                break;

            case Mode::SHADED:
            case Mode::TEXTURED:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_CULL_FACE);
                break;

            case Mode::SHADED_WIRE:
                // Sera rendu en deux passes dans le code principal
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_CULL_FACE);
                break;
        }
    }

    /**
     * @brief Pour le mode SHADED_WIRE, active le wireframe overlay
     */
    void enableWireframeOverlay() {
        if (m_currentMode == Mode::SHADED_WIRE) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);
        }
    }

    /**
     * @brief Désactive le wireframe overlay
     */
    void disableWireframeOverlay() {
        if (m_currentMode == Mode::SHADED_WIRE) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
            glDisable(GL_POLYGON_OFFSET_LINE);
        }
    }

private:
    RenderModeManager() : m_currentMode(Mode::TEXTURED) {}
    Mode m_currentMode;
};

/**
 * @brief Helper pour dessiner en mode SHADED_WIRE
 * Exemple d'utilisation dans main loop:
 * 
 * if (renderMode == Mode::SHADED_WIRE) {
 *     // Pass 1: Rendu normal avec textures
 *     renderScene(shader, objects);
 *     
 *     // Pass 2: Wireframe overlay
 *     RenderModeManager::getInstance().enableWireframeOverlay();
 *     shader.setVec3("wireframeColor", glm::vec3(0.0f, 0.0f, 0.0f));
 *     renderScene(shader, objects);
 *     RenderModeManager::getInstance().disableWireframeOverlay();
 * } else {
 *     renderScene(shader, objects);
 * }
 */