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
    
    // Get base color (from texture or material)
    vec3 materialDiffuse;
    if (useTextures && hasTexture) {
        materialDiffuse = texture(texture_diffuse1, TexCoord).rgb;
    } else {
        materialDiffuse = material.diffuse;
    }
    
    // Accumulate lighting from all sources
    vec3 result = vec3(0.0);
    for (int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
        result += calculateLight(lights[i], norm, viewDir, materialDiffuse);
    }
    
    // Gamma correction (optional but recommended)
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}