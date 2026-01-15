#pragma once
#include <glad/gl.h>
#include <string>

class Texture {
public:
    Texture() : m_textureID(0), m_width(0), m_height(0) {}
    
    Texture(const std::string& path) : m_textureID(0), m_width(0), m_height(0) {
        // TODO: Load texture from file using stb_image
    }
    
    ~Texture() {
        if (m_textureID) {
            glDeleteTextures(1, &m_textureID);
        }
    }
    
    void bind(unsigned int slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    }
    
    void unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    GLuint getID() const { return m_textureID; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    GLuint m_textureID;
    int m_width;
    int m_height;
};