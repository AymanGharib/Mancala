#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = loadShaderFile(vertexPath);
    std::string fragmentSource = loadShaderFile(fragmentPath);
    
    GLuint vertexID = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentID = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    linkProgram(vertexID, fragmentID);
    
    // Nettoyage des shaders individuels (déjà linkés)
    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
}

Shader::Shader(const char* vertexSource, const char* fragmentSource, bool fromSource) {
    GLuint vertexID = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentID = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    linkProgram(vertexID, fragmentID);
    
    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
}

Shader::~Shader() {
    glDeleteProgram(m_programID);
}

std::string Shader::loadShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::compileShader(const std::string& source, GLenum type) {
    GLuint shaderID = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shaderID, 1, &src, nullptr);
    glCompileShader(shaderID);
    
    checkCompileErrors(shaderID, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    
    return shaderID;
}

void Shader::linkProgram(GLuint vertexID, GLuint fragmentID) {
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexID);
    glAttachShader(m_programID, fragmentID);
    glLinkProgram(m_programID);
    
    checkCompileErrors(m_programID, "PROGRAM");
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Compilation error (" << type << "):\n" 
                      << infoLog << std::endl;
            throw std::runtime_error("Shader compilation failed");
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Linking error:\n" << infoLog << std::endl;
            throw std::runtime_error("Shader linking failed");
        }
    }
}

void Shader::use() const {
    glUseProgram(m_programID);
}

GLint Shader::getUniformLocation(const std::string& name) {
    // Cache pour éviter les appels glGetUniformLocation répétés
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    
    GLint location = glGetUniformLocation(m_programID, name.c_str());
    if (location == -1) {
        std::cerr << "[Shader] Warning: Uniform '" << name << "' not found" << std::endl;
    }
    
    m_uniformCache[name] = location;
    return location;
}

// ===== IMPLÉMENTATION SETTERS =====

void Shader::setBool(const std::string& name, bool value) {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}