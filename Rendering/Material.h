#pragma once
#include <glm/glm.hpp>

struct Material {
    glm::vec3 ambient{0.2f, 0.2f, 0.2f};
    glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float shininess{32.0f};
};