#pragma once
#include "core/Mesh.h"
#include "Rendering/Material.h"
#include "Transform.h"
#include "Rendering/shader.h"

class GameObject {
public:
    GameObject() : m_mesh(nullptr), m_visible(true) {}
    
    ~GameObject() {
        if (m_mesh) delete m_mesh;
    }

    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

    void setMesh(Mesh* mesh) { 
        if (m_mesh) delete m_mesh;
        m_mesh = mesh; 
    }
    
    void setMaterial(const Material& material) { 
        m_material = material; 
    }

    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    void render(Shader& shader) {
        if (!m_visible || !m_mesh) return;

        // Set material uniforms
        shader.setVec3("material.ambient", m_material.ambient);
        shader.setVec3("material.diffuse", m_material.diffuse);
        shader.setVec3("material.specular", m_material.specular);
        shader.setFloat("material.shininess", m_material.shininess);

        // Set model matrix - FIXED METHOD NAME
        shader.setMat4("model", m_transform.getModelMatrix());

        // Render mesh
        m_mesh->draw();
    }

private:
    Transform m_transform;
    Mesh* m_mesh;
    Material m_material;
    bool m_visible;
};