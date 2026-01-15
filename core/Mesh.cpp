#include "Mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

Mesh::Mesh() : m_VAO(0), m_VBO(0), m_EBO(0) {}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_vertices(vertices)
    , m_indices(indices)
    , m_VAO(0), m_VBO(0), m_EBO(0)
{
    setupMesh();
    calculateBounds();
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Mesh::setupMesh() {
    // Génération buffers
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    // VBO (vertices)
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 m_vertices.size() * sizeof(Vertex), 
                 m_vertices.data(), 
                 GL_STATIC_DRAW);
    
    // EBO (indices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 m_indices.size() * sizeof(unsigned int), 
                 m_indices.data(), 
                 GL_STATIC_DRAW);
    
    // Attributes layout
    // Location 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Location 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                          (void*)offsetof(Vertex, normal));
    
    // Location 2: TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                          (void*)offsetof(Vertex, texCoords));
    
    glBindVertexArray(0);
}

void Mesh::draw(unsigned int instanceCount) const {
    glBindVertexArray(m_VAO);
    
    if (instanceCount > 1) {
        glDrawElementsInstanced(GL_TRIANGLES, 
                               static_cast<GLsizei>(m_indices.size()), 
                               GL_UNSIGNED_INT, 
                               0, 
                               instanceCount);
    } else {
        glDrawElements(GL_TRIANGLES, 
                      static_cast<GLsizei>(m_indices.size()), 
                      GL_UNSIGNED_INT, 
                      0);
    }
    
    glBindVertexArray(0);
}

void Mesh::updateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 m_vertices.size() * sizeof(Vertex), 
                 m_vertices.data(), 
                 GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 m_indices.size() * sizeof(unsigned int), 
                 m_indices.data(), 
                 GL_STATIC_DRAW);
    
    calculateBounds();
}

void Mesh::calculateNormals() {
    // Réinitialiser normales
    for (auto& v : m_vertices) {
        v.normal = glm::vec3(0.0f);
    }
    
    // Accumuler normales par triangle
    for (size_t i = 0; i < m_indices.size(); i += 3) {
        unsigned int i0 = m_indices[i];
        unsigned int i1 = m_indices[i + 1];
        unsigned int i2 = m_indices[i + 2];
        
        glm::vec3 v0 = m_vertices[i0].position;
        glm::vec3 v1 = m_vertices[i1].position;
        glm::vec3 v2 = m_vertices[i2].position;
        
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        
        m_vertices[i0].normal += normal;
        m_vertices[i1].normal += normal;
        m_vertices[i2].normal += normal;
    }
    
    // Normaliser
    for (auto& v : m_vertices) {
        v.normal = glm::normalize(v.normal);
    }
}

void Mesh::calculateBounds() {
    if (m_vertices.empty()) {
        m_boundsMin = m_boundsMax = glm::vec3(0.0f);
        return;
    }
    
    m_boundsMin = m_boundsMax = m_vertices[0].position;
    
    for (const auto& v : m_vertices) {
        m_boundsMin = glm::min(m_boundsMin, v.position);
        m_boundsMax = glm::max(m_boundsMax, v.position);
    }
}

// ===== GÉNÉRATEURS PROCÉDURAUX =====

Mesh Mesh::createCube(float size) {
    float s = size * 0.5f;
    
    std::vector<Vertex> vertices = {
        // Front face
        {{-s, -s,  s}, { 0,  0,  1}, {0, 0}},
        {{ s, -s,  s}, { 0,  0,  1}, {1, 0}},
        {{ s,  s,  s}, { 0,  0,  1}, {1, 1}},
        {{-s,  s,  s}, { 0,  0,  1}, {0, 1}},
        
        // Back face
        {{ s, -s, -s}, { 0,  0, -1}, {0, 0}},
        {{-s, -s, -s}, { 0,  0, -1}, {1, 0}},
        {{-s,  s, -s}, { 0,  0, -1}, {1, 1}},
        {{ s,  s, -s}, { 0,  0, -1}, {0, 1}},
        
        // Top face
        {{-s,  s,  s}, { 0,  1,  0}, {0, 0}},
        {{ s,  s,  s}, { 0,  1,  0}, {1, 0}},
        {{ s,  s, -s}, { 0,  1,  0}, {1, 1}},
        {{-s,  s, -s}, { 0,  1,  0}, {0, 1}},
        
        // Bottom face
        {{-s, -s, -s}, { 0, -1,  0}, {0, 0}},
        {{ s, -s, -s}, { 0, -1,  0}, {1, 0}},
        {{ s, -s,  s}, { 0, -1,  0}, {1, 1}},
        {{-s, -s,  s}, { 0, -1,  0}, {0, 1}},
        
        // Right face
        {{ s, -s,  s}, { 1,  0,  0}, {0, 0}},
        {{ s, -s, -s}, { 1,  0,  0}, {1, 0}},
        {{ s,  s, -s}, { 1,  0,  0}, {1, 1}},
        {{ s,  s,  s}, { 1,  0,  0}, {0, 1}},
        
        // Left face
        {{-s, -s, -s}, {-1,  0,  0}, {0, 0}},
        {{-s, -s,  s}, {-1,  0,  0}, {1, 0}},
        {{-s,  s,  s}, {-1,  0,  0}, {1, 1}},
        {{-s,  s, -s}, {-1,  0,  0}, {0, 1}},
    };
    
    std::vector<unsigned int> indices;
    for (int i = 0; i < 6; i++) {
        int base = i * 4;
        indices.insert(indices.end(), {
            base + 0, base + 1, base + 2,
            base + 2, base + 3, base + 0
        });
    }
    
    return Mesh(vertices, indices);
}

Mesh Mesh::createSphere(float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    const float PI = 3.14159265359f;
    
    // Génération vertices
    for (int lat = 0; lat <= segments; lat++) {
        float theta = lat * PI / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        
        for (int lon = 0; lon <= segments; lon++) {
            float phi = lon * 2.0f * PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            
            glm::vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            glm::vec3 position = normal * radius;
            glm::vec2 texCoord(1.0f - (float)lon / segments, 1.0f - (float)lat / segments);
            
            vertices.push_back(Vertex(position, normal, texCoord));
        }
    }
    
    // Génération indices
    for (int lat = 0; lat < segments; lat++) {
        for (int lon = 0; lon < segments; lon++) {
            int first = lat * (segments + 1) + lon;
            int second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    return Mesh(vertices, indices);
}