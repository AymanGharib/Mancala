#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Rendering/Material.h"

/**
 * @brief Système de thèmes visuels pour personnalisation
 * Permet de changer l'apparence globale du jeu
 */
class ThemeManager {
public:
    struct Theme {
        std::string name;
        
        // Board materials
        Material boardMaterial;
        Material pitMaterial;
        
        // Seed colors (4 variations)
        std::vector<glm::vec3> seedColors;
        
        // Lighting
        glm::vec3 ambientLight;
        std::vector<glm::vec3> lightColors;
        
        // Environment
        glm::vec3 backgroundColor;
        glm::vec3 fogColor;
        
        // Textures (optionnel)
        std::string boardTexture;
        std::string seedTexture;
    };

    static ThemeManager& getInstance() {
        static ThemeManager instance;
        return instance;
    }

    // Theme management
    void loadThemes();
    void setTheme(int index);
    void setTheme(const std::string& name);
    int getThemeCount() const { return m_themes.size(); }
    const Theme& getCurrentTheme() const { return m_themes[m_currentThemeIndex]; }
    int getCurrentThemeIndex() const { return m_currentThemeIndex; }
    const Theme& getTheme(int index) const { return m_themes[index]; }
    
    // Apply theme to objects
    void applyThemeToBoard(GameObject* board);
    void applyThemeToPit(GameObject* pit, int pitIndex);
    void applyThemeToSeed(GameObject* seed, int seedIndex);

private:
    ThemeManager() { loadThemes(); }
    
    void createDefaultThemes();
    
    std::vector<Theme> m_themes;
    int m_currentThemeIndex = 0;
};

// ============================================
// IMPLEMENTATION
// ============================================

inline void ThemeManager::loadThemes() {
    m_themes.clear();
    createDefaultThemes();
}

inline void ThemeManager::createDefaultThemes() {
    // ===== THEME 1: CLASSIC WOOD =====
    {
        Theme theme;
        theme.name = "Classic Wood";
        
        // Board: Dark wood
        theme.boardMaterial.ambient = glm::vec3(0.3f, 0.2f, 0.1f);
        theme.boardMaterial.diffuse = glm::vec3(0.5f, 0.35f, 0.2f);
        theme.boardMaterial.specular = glm::vec3(0.2f);
        theme.boardMaterial.shininess = 32.0f;
        
        // Pits: Lighter wood
        theme.pitMaterial.ambient = glm::vec3(0.4f, 0.3f, 0.2f);
        theme.pitMaterial.diffuse = glm::vec3(0.6f, 0.45f, 0.3f);
        theme.pitMaterial.specular = glm::vec3(0.3f);
        theme.pitMaterial.shininess = 64.0f;
        
        // Seeds: Natural stone colors
        theme.seedColors = {
            glm::vec3(0.6f, 0.5f, 0.4f),  // Beige
            glm::vec3(0.5f, 0.4f, 0.3f),  // Brown
            glm::vec3(0.7f, 0.6f, 0.5f),  // Light tan
            glm::vec3(0.4f, 0.35f, 0.25f) // Dark brown
        };
        
        theme.ambientLight = glm::vec3(0.3f, 0.3f, 0.3f);
        theme.lightColors = {
            glm::vec3(1.0f, 0.95f, 0.85f)  // Warm white
        };
        
        theme.backgroundColor = glm::vec3(0.1f, 0.1f, 0.12f);
        theme.fogColor = glm::vec3(0.2f, 0.2f, 0.25f);
        
        m_themes.push_back(theme);
    }

    // ===== THEME 2: MODERN STONE =====
    {
        Theme theme;
        theme.name = "Modern Stone";
        
        // Board: Dark granite
        theme.boardMaterial.ambient = glm::vec3(0.15f, 0.15f, 0.18f);
        theme.boardMaterial.diffuse = glm::vec3(0.25f, 0.25f, 0.3f);
        theme.boardMaterial.specular = glm::vec3(0.5f);
        theme.boardMaterial.shininess = 128.0f;
        
        // Pits: Polished stone
        theme.pitMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.25f);
        theme.pitMaterial.diffuse = glm::vec3(0.35f, 0.35f, 0.4f);
        theme.pitMaterial.specular = glm::vec3(0.6f);
        theme.pitMaterial.shininess = 256.0f;
        
        // Seeds: Colorful glass
        theme.seedColors = {
            glm::vec3(0.2f, 0.5f, 0.8f),  // Blue
            glm::vec3(0.8f, 0.3f, 0.2f),  // Red
            glm::vec3(0.3f, 0.7f, 0.3f),  // Green
            glm::vec3(0.7f, 0.5f, 0.2f)   // Orange
        };
        
        theme.ambientLight = glm::vec3(0.2f, 0.2f, 0.25f);
        theme.lightColors = {
            glm::vec3(1.0f, 1.0f, 1.0f)  // Cool white
        };
        
        theme.backgroundColor = glm::vec3(0.05f, 0.05f, 0.08f);
        theme.fogColor = glm::vec3(0.1f, 0.1f, 0.15f);
        
        m_themes.push_back(theme);
    }

    // ===== THEME 3: EGYPTIAN GOLD =====
    {
        Theme theme;
        theme.name = "Egyptian Gold";
        
        // Board: Sandstone
        theme.boardMaterial.ambient = glm::vec3(0.5f, 0.4f, 0.2f);
        theme.boardMaterial.diffuse = glm::vec3(0.7f, 0.6f, 0.3f);
        theme.boardMaterial.specular = glm::vec3(0.4f);
        theme.boardMaterial.shininess = 64.0f;
        
        // Pits: Gold-rimmed
        theme.pitMaterial.ambient = glm::vec3(0.6f, 0.5f, 0.3f);
        theme.pitMaterial.diffuse = glm::vec3(0.8f, 0.7f, 0.4f);
        theme.pitMaterial.specular = glm::vec3(0.8f, 0.7f, 0.3f);
        theme.pitMaterial.shininess = 128.0f;
        
        // Seeds: Jewel-like
        theme.seedColors = {
            glm::vec3(0.1f, 0.3f, 0.7f),  // Sapphire
            glm::vec3(0.7f, 0.1f, 0.1f),  // Ruby
            glm::vec3(0.1f, 0.6f, 0.2f),  // Emerald
            glm::vec3(0.6f, 0.4f, 0.8f)   // Amethyst
        };
        
        theme.ambientLight = glm::vec3(0.4f, 0.35f, 0.2f);
        theme.lightColors = {
            glm::vec3(1.0f, 0.9f, 0.7f)  // Golden light
        };
        
        theme.backgroundColor = glm::vec3(0.15f, 0.12f, 0.08f);
        theme.fogColor = glm::vec3(0.3f, 0.25f, 0.15f);
        
        m_themes.push_back(theme);
    }

    // ===== THEME 4: NEON CYBER =====
    {
        Theme theme;
        theme.name = "Neon Cyber";
        
        // Board: Dark metal
        theme.boardMaterial.ambient = glm::vec3(0.05f, 0.05f, 0.08f);
        theme.boardMaterial.diffuse = glm::vec3(0.1f, 0.1f, 0.15f);
        theme.boardMaterial.specular = glm::vec3(0.9f, 0.9f, 1.0f);
        theme.boardMaterial.shininess = 256.0f;
        
        // Pits: Glowing edges
        theme.pitMaterial.ambient = glm::vec3(0.1f, 0.1f, 0.2f);
        theme.pitMaterial.diffuse = glm::vec3(0.15f, 0.15f, 0.3f);
        theme.pitMaterial.specular = glm::vec3(1.0f);
        theme.pitMaterial.shininess = 512.0f;
        
        // Seeds: Neon colors
        theme.seedColors = {
            glm::vec3(0.0f, 1.0f, 1.0f),  // Cyan
            glm::vec3(1.0f, 0.0f, 1.0f),  // Magenta
            glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow
            glm::vec3(0.0f, 1.0f, 0.0f)   // Green
        };
        
        theme.ambientLight = glm::vec3(0.1f, 0.1f, 0.15f);
        theme.lightColors = {
            glm::vec3(0.5f, 0.8f, 1.0f),  // Blue light
            glm::vec3(1.0f, 0.3f, 0.8f)   // Pink light
        };
        
        theme.backgroundColor = glm::vec3(0.0f, 0.0f, 0.05f);
        theme.fogColor = glm::vec3(0.05f, 0.05f, 0.15f);
        
        m_themes.push_back(theme);
    }
}

inline void ThemeManager::setTheme(int index) {
    if (index >= 0 && index < m_themes.size()) {
        m_currentThemeIndex = index;
    }
}

inline void ThemeManager::setTheme(const std::string& name) {
    for (size_t i = 0; i < m_themes.size(); ++i) {
        if (m_themes[i].name == name) {
            m_currentThemeIndex = i;
            return;
        }
    }
}

inline void ThemeManager::applyThemeToBoard(GameObject* board) {
    if (board) {
        board->setMaterial(getCurrentTheme().boardMaterial);
    }
}

inline void ThemeManager::applyThemeToPit(GameObject* pit, int pitIndex) {
    if (pit) {
        pit->setMaterial(getCurrentTheme().pitMaterial);
    }
}

inline void ThemeManager::applyThemeToSeed(GameObject* seed, int seedIndex) {
    if (seed) {
        const auto& theme = getCurrentTheme();
        Material mat;
        
        // Use cyclic color from theme
        glm::vec3 color = theme.seedColors[seedIndex % theme.seedColors.size()];
        
        mat.ambient = color * 0.3f;
        mat.diffuse = color;
        mat.specular = glm::vec3(0.8f);
        mat.shininess = 64.0f;
        
        seed->setMaterial(mat);
    }
}