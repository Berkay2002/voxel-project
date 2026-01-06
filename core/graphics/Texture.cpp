#include "Texture.h"
#include "../Logger.h"

#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Core {

Texture::Texture(const std::string &path) {
  // Load image with stb_image
  stbi_set_flip_vertically_on_load(true); // OpenGL expects bottom-left origin

  // Force loading with 4 channels (RGBA) to ensure alpha transparency works
  // This handles PNGs with transparency properly
  int originalChannels = 0;
  unsigned char *data =
      stbi_load(path.c_str(), &m_Width, &m_Height, &originalChannels, 4);
  m_Channels = 4; // We requested 4 channels

  if (!data) {
    LogError("Failed to load texture: " + path);
    return;
  }

  // Always use RGBA format since we forced 4 channels
  GLenum internalFormat = GL_RGBA8;
  GLenum dataFormat = GL_RGBA;

  // Create OpenGL texture
  glGenTextures(1, &m_ID);
  glBindTexture(GL_TEXTURE_2D, m_ID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST); // Pixelated look for voxels

  // Upload texture data
  glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), m_Width,
               m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free image data
  stbi_image_free(data);

  LogInfo("Texture loaded: " + path + " (" + std::to_string(m_Width) + "x" +
          std::to_string(m_Height) + ", " + std::to_string(m_Channels) +
          " channels)");
}

Texture::~Texture() {
  if (m_ID != 0) {
    glDeleteTextures(1, &m_ID);
  }
}

Texture::Texture(Texture &&other) noexcept
    : m_ID(other.m_ID), m_Width(other.m_Width), m_Height(other.m_Height),
      m_Channels(other.m_Channels) {
  other.m_ID = 0;
  other.m_Width = 0;
  other.m_Height = 0;
  other.m_Channels = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    if (m_ID != 0) {
      glDeleteTextures(1, &m_ID);
    }
    m_ID = other.m_ID;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_Channels = other.m_Channels;
    other.m_ID = 0;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_Channels = 0;
  }
  return *this;
}

void Texture::Bind(unsigned int slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

} // namespace Core
