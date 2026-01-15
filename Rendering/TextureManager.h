#pragma once

#include <glad/gl.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/vec3.hpp>

#include <memory>

/**
 * @brief Gestionnaire de textures avec cache
 * Charge et gère les textures OpenGL
 * Utilise stb_image pour le chargement
 */
class TextureManager {
public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }

    /**
     * @brief Charge une texture depuis un fichier
     * @param filepath Chemin vers le fichier image
     * @param srgb Si true, utilise GL_SRGB pour gamma correction
     * @return ID de texture OpenGL (0 si échec)
     */
    GLuint loadTexture(const std::string& filepath, bool srgb = false);

    /**
     * @brief Charge une cubemap pour skybox/reflections
     * @param faces 6 chemins dans l'ordre: +X,-X,+Y,-Y,+Z,-Z
     * @return ID de texture cubemap
     */
    GLuint loadCubemap(const std::vector<std::string>& faces);

    /**
     * @brief Récupère une texture déjà chargée
     * @param filepath Chemin du fichier
     * @return ID de texture (0 si non trouvée)
     */
    GLuint getTexture(const std::string& filepath) const;

    /**
     * @brief Crée une texture procédurale (damier, bruit, etc.)
     */
    GLuint createCheckerboardTexture(int size, glm::vec3 color1, glm::vec3 color2);
    GLuint createSolidColorTexture(glm::vec3 color);

    /**
     * @brief Libère toutes les textures
     */
    void cleanup();

    /**
     * @brief Libère une texture spécifique
     */
    void releaseTexture(const std::string& filepath);

private:
    TextureManager() = default;
    ~TextureManager() { cleanup(); }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    GLuint loadTextureFromFile(const std::string& filepath, bool srgb);

    std::unordered_map<std::string, GLuint> m_textureCache;
};

// ============================================
// IMPLEMENTATION (à mettre dans TextureManager.cpp)
// ============================================

// Inclure stb_image pour le chargement d'images
// #define STB_IMAGE_IMPLEMENTATION doit être dans UN SEUL .cpp
#include <stb_image.h>
#include <iostream>

inline GLuint TextureManager::loadTexture(const std::string& filepath, bool srgb) {
    // Vérifier si déjà chargée
    auto it = m_textureCache.find(filepath);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // Charger nouvelle texture
    GLuint textureID = loadTextureFromFile(filepath, srgb);
    if (textureID != 0) {
        m_textureCache[filepath] = textureID;
    }

    return textureID;
}

inline GLuint TextureManager::loadTextureFromFile(const std::string& filepath, bool srgb) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "[TextureManager] Failed to load texture: " << filepath << std::endl;
        return 0;
    }

    GLenum internalFormat = GL_RGB;
    GLenum dataFormat = GL_RGB;

    if (nrChannels == 1) {
        internalFormat = dataFormat = GL_RED;
    } else if (nrChannels == 3) {
        internalFormat = srgb ? GL_SRGB : GL_RGB;
        dataFormat = GL_RGB;
    } else if (nrChannels == 4) {
        internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, 
                 dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Paramètres de texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Anisotropic filtering (si supporté)
    float maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);

    stbi_image_free(data);

    std::cout << "[TextureManager] Loaded: " << filepath 
              << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;

    return textureID;
}

inline GLuint TextureManager::createCheckerboardTexture(int size, glm::vec3 color1, glm::vec3 color2) {
    std::vector<unsigned char> data(size * size * 3);
    
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool isColor1 = ((x / 16) + (y / 16)) % 2 == 0;
            glm::vec3 color = isColor1 ? color1 : color2;
            
            int idx = (y * size + x) * 3;
            data[idx + 0] = static_cast<unsigned char>(color.r * 255);
            data[idx + 1] = static_cast<unsigned char>(color.g * 255);
            data[idx + 2] = static_cast<unsigned char>(color.b * 255);
        }
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

inline GLuint TextureManager::createSolidColorTexture(glm::vec3 color) {
    unsigned char data[3] = {
        static_cast<unsigned char>(color.r * 255),
        static_cast<unsigned char>(color.g * 255),
        static_cast<unsigned char>(color.b * 255)
    };

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

inline GLuint TextureManager::getTexture(const std::string& filepath) const {
    auto it = m_textureCache.find(filepath);
    return (it != m_textureCache.end()) ? it->second : 0;
}

inline void TextureManager::cleanup() {
    for (auto& pair : m_textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    m_textureCache.clear();
}

inline void TextureManager::releaseTexture(const std::string& filepath) {
    auto it = m_textureCache.find(filepath);
    if (it != m_textureCache.end()) {
        glDeleteTextures(1, &it->second);
        m_textureCache.erase(it);
    }
}