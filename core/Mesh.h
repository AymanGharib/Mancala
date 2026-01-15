#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

/**
 * @struct Vertex
 * @brief Structure vertex complète
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    
    Vertex(const glm::vec3& pos = glm::vec3(0.0f),
           const glm::vec3& norm = glm::vec3(0.0f, 1.0f, 0.0f),
           const glm::vec2& uv = glm::vec2(0.0f))
        : position(pos), normal(norm), texCoords(uv) {}
};

/**
 * @class Mesh
 * @brief Représentation géométrique optimisée GPU
 * 
 * Stocke vertices, indices et gère VAO/VBO/EBO.
 * Supporte import OBJ, génération procédurale et instancing.
 */
class Mesh {
public:
    /**
     * @brief Constructeur par défaut
     */
    Mesh();
    
    /**
     * @brief Constructeur avec données
     */
    Mesh(const std::vector<Vertex>& vertices, 
         const std::vector<unsigned int>& indices);
    
    /**
     * @brief Destructeur - Libère VAO/VBO/EBO
     */
    ~Mesh();

    // Non-copiable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    /**
     * @brief Dessine le mesh
     * @param instanceCount Nombre d'instances (1 = standard, >1 = instancing)
     */
    void draw(unsigned int instanceCount = 1) const;

    /**
     * @brief Met à jour les données GPU
     */
    void updateBuffers();

    /**
     * @brief Calcule les normales automatiquement
     */
    void calculateNormals();

    /**
     * @brief Calcule la bounding box
     */
    void calculateBounds();

    // ===== GÉNÉRATEURS PROCÉDURAUX =====
    
    static Mesh createCube(float size = 1.0f);
    static Mesh createSphere(float radius = 1.0f, int segments = 32);
    static Mesh createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32);
    static Mesh createPlane(float width = 10.0f, float depth = 10.0f);
    
    // ===== IMPORT =====
    
    /**
     * @brief Charge un mesh depuis un fichier OBJ
     */
    static Mesh loadFromOBJ(const std::string& path);

    // ===== ACCESSEURS =====
    
    std::vector<Vertex>& getVertices() { return m_vertices; }
    std::vector<unsigned int>& getIndices() { return m_indices; }
    
    glm::vec3 getBoundsMin() const { return m_boundsMin; }
    glm::vec3 getBoundsMax() const { return m_boundsMax; }
    glm::vec3 getBoundsCenter() const { return (m_boundsMin + m_boundsMax) * 0.5f; }
    float getBoundingRadius() const { return glm::length(m_boundsMax - m_boundsMin) * 0.5f; }

private:
    /**
     * @brief Configure VAO/VBO/EBO
     */
    void setupMesh();

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    
    // OpenGL handles
    GLuint m_VAO, m_VBO, m_EBO;
    
    // Bounding volume
    glm::vec3 m_boundsMin;
    glm::vec3 m_boundsMax;
};