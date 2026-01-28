#version 330 core

#define MAX_LIGHTS 4

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// Uniforms
uniform vec3 viewPos;
uniform int numLights;
uniform Light lights[MAX_LIGHTS];
uniform Material material;

uniform bool useTextures;
uniform sampler2D texture_diffuse1;
uniform bool hasTexture;

// NEW: lighting toggle
uniform bool lightingEnabled;

vec3 calculateLight(Light light, vec3 normal, vec3 viewDir, vec3 materialDiffuse) {
    // Ambient
    vec3 ambient = light.color * material.ambient;

    // Diffuse
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * (diff * materialDiffuse);

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * (spec * material.specular);

    // Attenuation (distance falloff)
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * distance * distance);

    return (ambient + diffuse + specular) * light.intensity * attenuation;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Base color (texture or material)
    vec3 baseColor;
    if (useTextures && hasTexture) {
        baseColor = texture(texture_diffuse1, TexCoord).rgb;
    } else {
        baseColor = material.diffuse;
    }

    // ===== LIGHTING OFF: show unlit/flat shading (very clear difference) =====
    if (!lightingEnabled || numLights <= 0) {
        // Option A: pure base color (flat)
        vec3 result = baseColor;

        // (Optional) tiny ambient to avoid looking "too dead"
        // vec3 result = baseColor * 0.9 + material.ambient * 0.1;

        // Keep gamma correction consistent with lighting-on path
        result = pow(result, vec3(1.0 / 2.2));
        FragColor = vec4(result, 1.0);
        return;
    }

    // ===== LIGHTING ON: normal Blinn-Phong =====
    vec3 result = vec3(0.0);
    for (int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
        result += calculateLight(lights[i], norm, viewDir, baseColor);
    }

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
