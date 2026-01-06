#include "core/SSAO.h"
#include "core/Logger.h"
#include "core/graphics/Shader.h"

#include <glad/gl.h>
#include <random>
#include <cmath>

namespace Core {

SSAO::~SSAO() {
    CleanupResources();
}

void SSAO::CleanupResources() {
    // Delete FBOs
    if (m_DepthFBO) {
        glDeleteFramebuffers(1, &m_DepthFBO);
        m_DepthFBO = 0;
    }
    if (m_SSAOFBO) {
        glDeleteFramebuffers(1, &m_SSAOFBO);
        m_SSAOFBO = 0;
    }
    if (m_BlurFBO) {
        glDeleteFramebuffers(1, &m_BlurFBO);
        m_BlurFBO = 0;
    }

    // Delete textures
    if (m_DepthTexture) {
        glDeleteTextures(1, &m_DepthTexture);
        m_DepthTexture = 0;
    }
    if (m_NormalTexture) {
        glDeleteTextures(1, &m_NormalTexture);
        m_NormalTexture = 0;
    }
    if (m_SSAOTexture) {
        glDeleteTextures(1, &m_SSAOTexture);
        m_SSAOTexture = 0;
    }
    if (m_BlurTexture) {
        glDeleteTextures(1, &m_BlurTexture);
        m_BlurTexture = 0;
    }
    if (m_NoiseTexture) {
        glDeleteTextures(1, &m_NoiseTexture);
        m_NoiseTexture = 0;
    }

    // Delete quad VAO/VBO
    if (m_QuadVAO) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        m_QuadVAO = 0;
    }
    if (m_QuadVBO) {
        glDeleteBuffers(1, &m_QuadVBO);
        m_QuadVBO = 0;
    }
}

bool SSAO::Setup(int width, int height) {
    m_Width = width;
    m_Height = height;
    m_SSAOWidth = width / 2;
    m_SSAOHeight = height / 2;

    // Load shaders
    m_DepthShader = std::make_unique<Shader>(
        "assets/shaders/depth_normal.vert",
        "assets/shaders/depth_normal.frag"
    );
    if (!m_DepthShader->IsValid()) {
        LOG_ERROR("SSAO: Failed to load depth_normal shader");
        return false;
    }

    m_SSAOShader = std::make_unique<Shader>(
        "assets/shaders/fullscreen.vert",
        "assets/shaders/ssao.frag"
    );
    if (!m_SSAOShader->IsValid()) {
        LOG_ERROR("SSAO: Failed to load ssao shader");
        return false;
    }

    m_BlurShader = std::make_unique<Shader>(
        "assets/shaders/fullscreen.vert",
        "assets/shaders/ssao_blur.frag"
    );
    if (!m_BlurShader->IsValid()) {
        LOG_ERROR("SSAO: Failed to load ssao_blur shader");
        return false;
    }

    // ========================================================================
    // Depth Pre-pass FBO (full resolution)
    // ========================================================================
    glGenFramebuffers(1, &m_DepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);

    // Depth texture
    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

    // Normal texture (view-space normals)
    glGenTextures(1, &m_NormalTexture);
    glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
                 GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_NormalTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("SSAO: Depth FBO not complete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    // ========================================================================
    // SSAO FBO (half resolution)
    // ========================================================================
    glGenFramebuffers(1, &m_SSAOFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);

    glGenTextures(1, &m_SSAOTexture);
    glBindTexture(GL_TEXTURE_2D, m_SSAOTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_SSAOWidth, m_SSAOHeight, 0,
                 GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("SSAO: SSAO FBO not complete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    // ========================================================================
    // Blur FBO (half resolution)
    // ========================================================================
    glGenFramebuffers(1, &m_BlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBO);

    glGenTextures(1, &m_BlurTexture);
    glBindTexture(GL_TEXTURE_2D, m_BlurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_SSAOWidth, m_SSAOHeight, 0,
                 GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // Linear for smooth sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BlurTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("SSAO: Blur FBO not complete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate kernel and noise
    GenerateKernel();
    GenerateNoiseTexture();
    SetupFullscreenQuad();

    LOG_INFO("SSAO: Initialized at " + std::to_string(width) + "x" + std::to_string(height) +
             " (SSAO at " + std::to_string(m_SSAOWidth) + "x" + std::to_string(m_SSAOHeight) + ")");

    return true;
}

void SSAO::Resize(int width, int height) {
    if (width == m_Width && height == m_Height) return;

    // Store new dimensions
    m_Width = width;
    m_Height = height;
    m_SSAOWidth = width / 2;
    m_SSAOHeight = height / 2;

    // Resize depth texture
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Resize normal texture
    glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
                 GL_RGB, GL_FLOAT, nullptr);

    // Resize SSAO texture
    glBindTexture(GL_TEXTURE_2D, m_SSAOTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_SSAOWidth, m_SSAOHeight, 0,
                 GL_RED, GL_FLOAT, nullptr);

    // Resize blur texture
    glBindTexture(GL_TEXTURE_2D, m_BlurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_SSAOWidth, m_SSAOHeight, 0,
                 GL_RED, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_INFO("SSAO: Resized to " + std::to_string(width) + "x" + std::to_string(height));
}

void SSAO::GenerateKernel() {
    // Generate 64 sample points in a hemisphere
    // Using cosine-weighted distribution for better quality
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;

    m_Kernel.clear();
    m_Kernel.reserve(64);

    for (int i = 0; i < 64; ++i) {
        // Random point in hemisphere (normal space, z >= 0)
        glm::vec3 sample(
            randomFloats(generator) * 2.0f - 1.0f,  // x: [-1, 1]
            randomFloats(generator) * 2.0f - 1.0f,  // y: [-1, 1]
            randomFloats(generator)                  // z: [0, 1] (hemisphere)
        );
        sample = glm::normalize(sample);

        // Scale sample to be within unit hemisphere
        sample *= randomFloats(generator);

        // Accelerating interpolation (more samples close to origin)
        // This gives better quality for nearby occlusion
        float scale = static_cast<float>(i) / 64.0f;
        scale = 0.1f + scale * scale * (1.0f - 0.1f);  // lerp(0.1, 1.0, scale^2)
        sample *= scale;

        m_Kernel.push_back(sample);
    }
}

void SSAO::GenerateNoiseTexture() {
    // Generate 4x4 noise texture with random rotation vectors
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;

    std::vector<glm::vec3> noise;
    noise.reserve(16);

    for (int i = 0; i < 16; ++i) {
        // Random rotation around Z axis (in tangent space)
        glm::vec3 rotationVec(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f  // Z = 0 because we only rotate around surface normal
        );
        noise.push_back(rotationVec);
    }

    glGenTextures(1, &m_NoiseTexture);
    glBindTexture(GL_TEXTURE_2D, m_NoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Tile across screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SSAO::SetupFullscreenQuad() {
    // Fullscreen quad vertices (position + texcoord)
    float quadVertices[] = {
        // Position    // TexCoord
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);

    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TexCoord attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void SSAO::BeginDepthPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
    glViewport(0, 0, m_Width, m_Height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void SSAO::EndDepthPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::Calculate(const glm::mat4& projection, const glm::mat4& view) {
    (void)view;  // View matrix not needed here, normals already in view space

    // Calculate inverse projection for depth reconstruction
    glm::mat4 invProjection = glm::inverse(projection);

    // ========================================================================
    // SSAO Pass (half resolution)
    // ========================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
    glViewport(0, 0, m_SSAOWidth, m_SSAOHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    m_SSAOShader->Bind();

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    m_SSAOShader->SetInt("u_DepthTex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
    m_SSAOShader->SetInt("u_NormalTex", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_NoiseTexture);
    m_SSAOShader->SetInt("u_NoiseTex", 2);

    // Set uniforms
    m_SSAOShader->SetMat4("u_Projection", projection);
    m_SSAOShader->SetMat4("u_InvProjection", invProjection);
    m_SSAOShader->SetVec2("u_NoiseScale", glm::vec2(
        static_cast<float>(m_SSAOWidth) / 4.0f,
        static_cast<float>(m_SSAOHeight) / 4.0f
    ));

    // SSAO parameters (from WorldConfig)
    m_SSAOShader->SetFloat("u_Radius", 0.5f);   // SSAO_RADIUS
    m_SSAOShader->SetFloat("u_Bias", 0.025f);   // SSAO_BIAS
    m_SSAOShader->SetFloat("u_Power", 2.0f);    // SSAO_POWER

    // Upload kernel samples
    for (int i = 0; i < 64; ++i) {
        m_SSAOShader->SetVec3("u_Samples[" + std::to_string(i) + "]", m_Kernel[i]);
    }

    // Render fullscreen quad
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // ========================================================================
    // Blur Pass (half resolution)
    // ========================================================================
    glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    m_BlurShader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_SSAOTexture);
    m_BlurShader->SetInt("u_SSAOTex", 0);

    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::BindAOTexture(int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_BlurTexture);
}

void SSAO::UnbindAOTexture(int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Core
