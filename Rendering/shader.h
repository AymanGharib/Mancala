#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

/**
 * @class Shader
 * @brief Gestion des programmes shader OpenGL
 * 
 * Fonctionnalités :
 * - Compilation vertex/fragment shaders
 * - Linkage du programme
 * - Gestion des uniformes avec cache
 * - Détection d'erreurs
 */
class Shader {
public:
    /**
     * @brief Constructeur - Compile et linke les shaders
     * @param vertexPath Chemin du vertex shader
     * @param fragmentPath Chemin du fragment shader
     */
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    
    /**
     * @brief Constructeur - À partir du code source directement
     */
    Shader(const char* vertexSource, const char* fragmentSource, bool fromSource);

    /**
     * @brief Destructeur - Libère le programme GPU
     */
    ~Shader();

    // Non-copiable (ressource GPU unique)
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    /**
     * @brief Active ce shader pour le rendu
     */
    void use() const;

    /**
     * @brief Obtient l'ID du programme
     */
    GLuint getID() const { return m_programID; }

    // ===== SETTERS UNIFORMES (avec cache de locations) =====
    
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    
    void setMat3(const std::string& name, const glm::mat3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    /**
     * @brief Compile un shader individuel
     * @param source Code source GLSL
     * @param type GL_VERTEX_SHADER ou GL_FRAGMENT_SHADER
     * @return ID du shader compilé
     */
    GLuint compileShader(const std::string& source, GLenum type);

    /**
     * @brief Linke vertex + fragment en programme
     */
    void linkProgram(GLuint vertexID, GLuint fragmentID);

    /**
     * @brief Vérifie les erreurs de compilation/linkage
     */
    void checkCompileErrors(GLuint shader, const std::string& type);

    /**
     * @brief Obtient la location d'un uniforme (avec cache)
     */
    GLint getUniformLocation(const std::string& name);

    /**
     * @brief Charge le contenu d'un fichier shader
     */
    std::string loadShaderFile(const std::string& path);

    GLuint m_programID;
    std::unordered_map<std::string, GLint> m_uniformCache;
};