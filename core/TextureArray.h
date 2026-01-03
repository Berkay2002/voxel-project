#pragma once

#include <string>
#include <vector>

namespace Core {

/**
 * TextureArray wraps an OpenGL GL_TEXTURE_2D_ARRAY for efficient
 * multi-texture rendering. Each texture file becomes a layer in the array.
 * All textures must have the same dimensions.
 */
class TextureArray {
public:
    /**
     * Create a texture array from a list of image paths.
     * All images must have the same width and height.
     * @param paths Vector of file paths to load as array layers
     */
    explicit TextureArray(const std::vector<std::string>& paths);
    ~TextureArray();

    // Non-copyable
    TextureArray(const TextureArray&) = delete;
    TextureArray& operator=(const TextureArray&) = delete;

    // Movable
    TextureArray(TextureArray&& other) noexcept;
    TextureArray& operator=(TextureArray&& other) noexcept;

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    [[nodiscard]] unsigned int GetID() const { return m_ID; }
    [[nodiscard]] int GetWidth() const { return m_Width; }
    [[nodiscard]] int GetHeight() const { return m_Height; }
    [[nodiscard]] int GetLayerCount() const { return m_LayerCount; }
    [[nodiscard]] bool IsValid() const { return m_ID != 0; }

private:
    unsigned int m_ID = 0;
    int m_Width = 0;
    int m_Height = 0;
    int m_LayerCount = 0;
};

} // namespace Core
