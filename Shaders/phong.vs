#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex position
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal (use normal matrix to handle non-uniform scaling)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass texture coordinates
    TexCoord = aTexCoord;
    
    // Final position
    gl_Position = projection * view * vec4(FragPos, 1.0);
}