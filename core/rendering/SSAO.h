/**
 * SSAO.h - Screen-Space Ambient Occlusion
 * 
 * Implements SSAO using a forward + depth pre-pass approach:
 * 1. Depth Pre-pass: Render scene to depth + view-space normal textures
 * 2. SSAO Pass: Sample hemisphere around each fragment, compare depths
 * 3. Blur Pass: 5x5 box blur to smooth the result
 * 4. Final: Sample blurred AO texture in lit.frag
 * 
 * Key decision: Half-resolution SSAO for performance (looks good with blur)
 */

#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Core {

class Shader;

class SSAO {
public:
    SSAO() = default;
    ~SSAO();

    // Non-copyable, non-movable
    SSAO(const SSAO&) = delete;
    SSAO& operator=(const SSAO&) = delete;
    SSAO(SSAO&&) = delete;
    SSAO& operator=(SSAO&&) = delete;

    /**
     * Initialize SSAO system
     * @param width Full-resolution width
     * @param height Full-resolution height
     * @return true if setup succeeded
     */
    bool Setup(int width, int height);

    /**
     * Resize textures when window size changes
     */
    void Resize(int width, int height);

    /**
     * Begin depth pre-pass (call before rendering chunks)
     * Sets up FBO, clears depth and normal buffers
     */
    void BeginDepthPass();

    /**
     * End depth pre-pass (restore default framebuffer)
     */
    void EndDepthPass();

    /**
     * Get the depth+normal shader for rendering chunks
     */
    Shader* GetDepthShader() { return m_DepthShader.get(); }

    /**
     * Calculate SSAO from depth buffer
     * Runs SSAO calculation pass + blur pass
     * @param projection Camera projection matrix
     * @param view Camera view matrix
     */
    void Calculate(const glm::mat4& projection, const glm::mat4& view);

    /**
     * Bind blurred AO texture for sampling in lit.frag
     * @param slot Texture unit slot
     */
    void BindAOTexture(int slot);

    /**
     * Unbind AO texture
     */
    void UnbindAOTexture(int slot);

    // Toggle
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    [[nodiscard]] bool IsEnabled() const { return m_Enabled; }

    // Accessors for viewport restore
    [[nodiscard]] int GetWidth() const { return m_Width; }
    [[nodiscard]] int GetHeight() const { return m_Height; }

private:
    void GenerateKernel();
    void GenerateNoiseTexture();
    void SetupFullscreenQuad();
    void CleanupResources();

    // Shaders
    std::unique_ptr<Shader> m_DepthShader;   // Depth pre-pass
    std::unique_ptr<Shader> m_SSAOShader;    // SSAO calculation
    std::unique_ptr<Shader> m_BlurShader;    // Box blur

    // Depth pre-pass FBO (full resolution)
    unsigned int m_DepthFBO = 0;
    unsigned int m_DepthTexture = 0;         // GL_DEPTH_COMPONENT24
    unsigned int m_NormalTexture = 0;        // GL_RGB16F (view-space normals)

    // SSAO FBO (half-resolution)
    unsigned int m_SSAOFBO = 0;
    unsigned int m_SSAOTexture = 0;          // GL_R8 (single-channel AO)

    // Blur FBO (half-resolution)
    unsigned int m_BlurFBO = 0;
    unsigned int m_BlurTexture = 0;          // GL_R8 (blurred AO)

    // Noise texture (4x4 rotation vectors)
    unsigned int m_NoiseTexture = 0;

    // Sample kernel (64 hemisphere samples)
    std::vector<glm::vec3> m_Kernel;

    // Fullscreen quad for post-processing
    unsigned int m_QuadVAO = 0;
    unsigned int m_QuadVBO = 0;

    // Dimensions
    int m_Width = 0;
    int m_Height = 0;
    int m_SSAOWidth = 0;   // Half of m_Width
    int m_SSAOHeight = 0;  // Half of m_Height

    bool m_Enabled = true;
};

} // namespace Core
