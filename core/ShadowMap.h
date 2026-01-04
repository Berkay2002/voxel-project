/**
 * ShadowMap.h - Depth-only framebuffer for shadow mapping
 * 
 * Creates a depth texture rendered from the sun's perspective that can be
 * sampled during the main pass to determine which fragments are in shadow.
 * 
 * Usage:
 *   1. Create() - Initialize FBO with depth texture
 *   2. Bind() - Render shadow-casting geometry from light's POV
 *   3. Unbind() - Restore default framebuffer
 *   4. BindTexture() - Sample in main pass fragment shader
 */

#pragma once

namespace Core {

class ShadowMap {
public:
    ShadowMap() = default;
    ~ShadowMap();

    // Non-copyable, non-movable
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    ShadowMap(ShadowMap&&) = delete;
    ShadowMap& operator=(ShadowMap&&) = delete;

    /**
     * Create the shadow map FBO and depth texture
     * @param width Texture width in pixels (recommend 2048 or 4096)
     * @param height Texture height in pixels (recommend same as width)
     * @return true if creation succeeded
     */
    bool Create(int width, int height);

    /**
     * Bind FBO for shadow pass rendering
     * Remember to set viewport to shadow map dimensions
     */
    void Bind();

    /**
     * Unbind and restore default framebuffer (id 0)
     */
    void Unbind();

    /**
     * Bind depth texture for sampling in fragment shader
     * @param slot Texture unit (e.g., GL_TEXTURE0 + slot)
     */
    void BindTexture(int slot);

    /**
     * Unbind the texture from the given slot
     */
    void UnbindTexture(int slot);

    [[nodiscard]] int GetWidth() const { return m_Width; }
    [[nodiscard]] int GetHeight() const { return m_Height; }
    [[nodiscard]] unsigned int GetDepthTexture() const { return m_DepthTexture; }

private:
    unsigned int m_FBO = 0;
    unsigned int m_DepthTexture = 0;
    int m_Width = 0;
    int m_Height = 0;
};

} // namespace Core
