#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <initializer_list>
#include "Rendering/Camera.h"
#include "Scene/GameObject.h"
#include <vector>

/**
 * @brief Système de sélection d'objets par ray casting
 * Convertit position souris 2D -> rayon 3D -> intersection avec objets
 */
class ObjectPicker {
public:
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    struct RayHit {
        GameObject* object;
        float distance;
        glm::vec3 hitPoint;
        bool hit;
    };

    ObjectPicker() = default;

    /**
     * @brief Crée un rayon depuis la position de la souris
     * @param mouseX Position X de la souris (coordonnées écran)
     * @param mouseY Position Y de la souris (coordonnées écran)
     * @param screenWidth Largeur de la fenêtre
     * @param screenHeight Hauteur de la fenêtre
     * @param camera Caméra actuelle
     * @return Rayon en espace monde
     */
    static Ray screenToWorldRay(
        float mouseX, 
        float mouseY,
        int screenWidth,
        int screenHeight,
        const Camera& camera
    );

    /**
     * @brief Teste l'intersection d'un rayon avec une sphère
     * @param ray Rayon à tester
     * @param center Centre de la sphère
     * @param radius Rayon de la sphère
     * @param outDistance Distance d'intersection (si hit)
     * @return true si intersection
     */
    static bool raySphereIntersection(
        const Ray& ray,
        const glm::vec3& center,
        float radius,
        float& outDistance
    );

    /**
     * @brief Teste l'intersection avec une boîte alignée (AABB)
     * @param ray Rayon à tester
     * @param min Coin minimum de la boîte
     * @param max Coin maximum de la boîte
     * @param outDistance Distance d'intersection
     * @return true si intersection
     */
    static bool rayAABBIntersection(
        const Ray& ray,
        const glm::vec3& min,
        const glm::vec3& max,
        float& outDistance
    );

    /**
     * @brief Trouve l'objet le plus proche touché par le rayon
     * @param ray Rayon à tester
     * @param objects Liste d'objets à tester
     * @return Information sur le hit (ou hit=false si aucun)
     */
    static RayHit pickObject(
        const Ray& ray,
        const std::vector<GameObject*>& objects
    );

    /**
     * @brief Trouve l'objet le plus proche (version simplifiée sphere)
     * Assume que tous les objets ont un collider sphérique
     */
    static GameObject* pickClosestSphere(
        const Ray& ray,
        const std::vector<GameObject*>& objects,
        float sphereRadius = 0.5f
    );

private:
    // Helper: Calcule l'AABB d'un GameObject basé sur son mesh
    static void getObjectBounds(
        GameObject* obj, 
        glm::vec3& outMin, 
        glm::vec3& outMax
    );
};

// ============================================
// IMPLEMENTATION
// ============================================

inline ObjectPicker::Ray ObjectPicker::screenToWorldRay(
    float mouseX,
    float mouseY,
    int screenWidth,
    int screenHeight,
    const Camera& camera
) {
    // 1. Convertir coordonnées écran en NDC [-1, 1]
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;  // Y inversé
    
    // 2. NDC -> Clip coordinates (z=-1 pour near plane, w=1)
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);
    
    // 3. Clip -> Eye coordinates (inverse projection)
    glm::mat4 projInverse = glm::inverse(camera.getProjectionMatrix());
    glm::vec4 rayEye = projInverse * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);  // Direction vers l'avant
    
    // 4. Eye -> World coordinates (inverse view)
    glm::mat4 viewInverse = glm::inverse(camera.getViewMatrix());
    glm::vec4 rayWorld4 = viewInverse * rayEye;
    glm::vec3 rayWorld = glm::normalize(glm::vec3(rayWorld4));
    
    Ray ray;
    ray.origin = camera.getPosition();
    ray.direction = rayWorld;
    return ray;
}

inline bool ObjectPicker::raySphereIntersection(
    const Ray& ray,
    const glm::vec3& center,
    float radius,
    float& outDistance
) {
    glm::vec3 oc = ray.origin - center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return false;  // Pas d'intersection
    }
    
    // Calculer la distance la plus proche
    float sqrtDisc = sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    
    // Prendre la plus petite distance positive
    if (t1 > 0) {
        outDistance = t1;
    } else if (t2 > 0) {
        outDistance = t2;
    } else {
        return false;  // Derrière la caméra
    }
    
    return true;
}

inline bool ObjectPicker::rayAABBIntersection(
    const Ray& ray,
    const glm::vec3& min,
    const glm::vec3& max,
    float& outDistance
) {
    float tmin = 0.0f;
    float tmax = FLT_MAX;
    
    for (int i = 0; i < 3; ++i) {
        if (abs(ray.direction[i]) < 1e-6f) {
            // Rayon parallèle à la face
            if (ray.origin[i] < min[i] || ray.origin[i] > max[i]) {
                return false;
            }
        } else {
            float t1 = (min[i] - ray.origin[i]) / ray.direction[i];
            float t2 = (max[i] - ray.origin[i]) / ray.direction[i];
            
            if (t1 > t2) std::swap(t1, t2);
            
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            
            if (tmin > tmax) return false;
        }
    }
    
    outDistance = tmin > 0 ? tmin : tmax;
    return outDistance > 0;
}

inline ObjectPicker::RayHit ObjectPicker::pickObject(
    const Ray& ray,
    const std::vector<GameObject*>& objects
) {
    RayHit closestHit;
    closestHit.hit = false;
    closestHit.distance = FLT_MAX;
    closestHit.object = nullptr;
    
    for (GameObject* obj : objects) {
        if (!obj || !obj->isVisible()) continue;
        
        glm::vec3 objPos = obj->getTransform().getPosition();
        glm::vec3 scale = obj->getTransform().getScale();
        
        // Utiliser le plus grand composant de scale comme rayon approximatif
        float radius = std::max({scale.x, scale.y, scale.z}) * 0.5f;
        
        float distance;
        if (raySphereIntersection(ray, objPos, radius, distance)) {
            if (distance < closestHit.distance) {
                closestHit.hit = true;
                closestHit.distance = distance;
                closestHit.object = obj;
                closestHit.hitPoint = ray.origin + ray.direction * distance;
            }
        }
    }
    
    return closestHit;
}

inline GameObject* ObjectPicker::pickClosestSphere(
    const Ray& ray,
    const std::vector<GameObject*>& objects,
    float sphereRadius
) {
    RayHit hit = pickObject(ray, objects);
    return hit.hit ? hit.object : nullptr;
}