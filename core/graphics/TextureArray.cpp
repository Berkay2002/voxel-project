#include "TextureArray.h"
#include "../Logger.h"

#include <glad/gl.h>
#include "stb_image.h"

namespace Core {

TextureArray::TextureArray(const std::vector<std::string>& paths) {
    if (paths.empty()) {
        LogError("TextureArray: No paths provided");
        return;
    }

    m_LayerCount = static_cast<int>(paths.size());

    // Load first image to get dimensions
    stbi_set_flip_vertically_on_load(true);
    int firstWidth, firstHeight, firstChannels;
    unsigned char* firstData = stbi_load(paths[0].c_str(), &firstWidth, &firstHeight, &firstChannels, 4);
    
    if (!firstData) {
        LogError("TextureArray: Failed to load first texture: " + paths[0]);
        return;
    }

    m_Width = firstWidth;
    m_Height = firstHeight;

    // Create OpenGL texture array
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);

    // Allocate storage for all layers (using RGBA8 for consistency)
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 
                 m_Width, m_Height, m_LayerCount, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Upload first layer
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, 0,  // x, y, layer offset
                    m_Width, m_Height, 1,  // width, height, depth
                    GL_RGBA, GL_UNSIGNED_BYTE, firstData);
    stbi_image_free(firstData);

    LogInfo("TextureArray layer 0: " + paths[0] + " (" + 
            std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");

    // Load and upload remaining layers
    for (int i = 1; i < m_LayerCount; ++i) {
        int width, height, channels;
        unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 4);
        
        if (!data) {
            LogError("TextureArray: Failed to load texture: " + paths[i]);
            continue;
        }

        if (width != m_Width || height != m_Height) {
            LogError("TextureArray: Dimension mismatch for " + paths[i] + 
                     " (expected " + std::to_string(m_Width) + "x" + std::to_string(m_Height) +
                     ", got " + std::to_string(width) + "x" + std::to_string(height) + ")");
            stbi_image_free(data);
            continue;
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                        0, 0, i,  // x, y, layer offset
                        m_Width, m_Height, 1,
                        GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        LogInfo("TextureArray layer " + std::to_string(i) + ": " + paths[i]);
    }

    // Set texture parameters (nearest filtering for pixel art look)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    LogInfo("TextureArray created with " + std::to_string(m_LayerCount) + " layers");
}

TextureArray::~TextureArray() {
    if (m_ID != 0) {
        glDeleteTextures(1, &m_ID);
    }
}

TextureArray::TextureArray(TextureArray&& other) noexcept
    : m_ID(other.m_ID), m_Width(other.m_Width), 
      m_Height(other.m_Height), m_LayerCount(other.m_LayerCount) {
    other.m_ID = 0;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_LayerCount = 0;
}

TextureArray& TextureArray::operator=(TextureArray&& other) noexcept {
    if (this != &other) {
        if (m_ID != 0) {
            glDeleteTextures(1, &m_ID);
        }
        m_ID = other.m_ID;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_LayerCount = other.m_LayerCount;
        other.m_ID = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_LayerCount = 0;
    }
    return *this;
}

void TextureArray::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);
}

void TextureArray::Unbind() const {
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

} // namespace Core
