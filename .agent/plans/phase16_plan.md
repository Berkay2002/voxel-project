# Phase 16: Screen-Space Ambient Occlusion (SSAO) Implementation Plan

## Overview

SSAO is a post-processing technique that darkens corners, crevices, and contact points between surfaces by sampling the depth buffer in screen space.

## Architecture

```
Main Pass → G-Buffer (Position, Normal, Depth)
                ↓
         SSAO Pass → AO Texture (single channel)
                ↓
         Blur Pass → Smoothed AO
                ↓
       Compose Pass → Final Image with AO applied
```

---

## Part A: G-Buffer Setup

### [NEW] core/GBuffer.h/.cpp

```cpp
class GBuffer {
public:
    bool Create(int width, int height);
    void Resize(int width, int height);
    void Bind();
    void Unbind();

    void BindTextures();  // Bind all textures for reading

    [[nodiscard]] unsigned int GetPositionTexture() const { return m_PositionTexture; }
    [[nodiscard]] unsigned int GetNormalTexture() const { return m_NormalTexture; }
    [[nodiscard]] unsigned int GetDepthTexture() const { return m_DepthTexture; }

private:
    unsigned int m_FBO = 0;
    unsigned int m_PositionTexture = 0;  // RGB32F - world-space position
    unsigned int m_NormalTexture = 0;    // RGB16F - world-space normal
    unsigned int m_AlbedoTexture = 0;    // RGBA8 - color + alpha
    unsigned int m_DepthTexture = 0;     // DEPTH24_STENCIL8
    int m_Width = 0, m_Height = 0;
};
```

**Texture Formats:**

- Position: `GL_RGB32F` (12 bytes/pixel)
- Normal: `GL_RGB16F` (6 bytes/pixel)
- Albedo: `GL_RGBA8` (4 bytes/pixel)
- Depth: `GL_DEPTH24_STENCIL8`

---

### [NEW] assets/shaders/gbuffer.vert

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
// ... other attributes

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 u_MVP;
uniform mat4 u_Model;

void main() {
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(u_Model) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = u_MVP * vec4(aPos, 1.0);
}
```

### [NEW] assets/shaders/gbuffer.frag

```glsl
#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2DArray u_TextureArray;
uniform float u_TexIndex;

void main() {
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo = texture(u_TextureArray, vec3(TexCoord, u_TexIndex));
}
```

---

## Part B: SSAO Pass

### [NEW] assets/shaders/ssao.frag

```glsl
#version 460 core

out float FragColor;

in vec2 TexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];  // Kernel samples
uniform mat4 projection;

// SSAO parameters
const int KERNEL_SIZE = 64;
const float RADIUS = 0.5;
const float BIAS = 0.025;

// Tile noise texture over screen (4x4 noise, repeat)
const vec2 noiseScale = vec2(1920.0/4.0, 1080.0/4.0);

void main() {
    vec3 fragPos = texture(gPosition, TexCoord).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoord).xyz);
    vec3 randomVec = normalize(texture(texNoise, TexCoord * noiseScale).xyz);

    // Create TBN matrix for hemisphere orientation
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Sample hemisphere and accumulate occlusion
    float occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        // Transform sample to world space
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * RADIUS;

        // Project sample to screen space
        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // Sample depth at this position
        float sampleDepth = texture(gPosition, offset.xy).z;

        // Range check and accumulate
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;
    }

    FragColor = 1.0 - (occlusion / float(KERNEL_SIZE));
}
```

---

### SSAO Kernel Generation (C++)

```cpp
std::vector<glm::vec3> GenerateSSAOKernel(int size) {
    std::vector<glm::vec3> kernel;
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;

    for (int i = 0; i < size; ++i) {
        // Random point in hemisphere
        glm::vec3 sample(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator)  // Hemisphere: z in [0, 1]
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        // Scale samples to cluster near origin (more samples close = better quality)
        float scale = static_cast<float>(i) / static_cast<float>(size);
        scale = glm::mix(0.1f, 1.0f, scale * scale);
        sample *= scale;

        kernel.push_back(sample);
    }
    return kernel;
}
```

---

### Noise Texture Generation

```cpp
std::vector<glm::vec3> GenerateSSAONoise(int size) {
    std::vector<glm::vec3> noise;
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;

    for (int i = 0; i < size * size; ++i) {
        // Random rotation around Z-axis
        glm::vec3 n(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f
        );
        noise.push_back(n);
    }
    return noise;
}
```

---

## Part C: Blur Pass

### [NEW] assets/shaders/ssao_blur.frag

```glsl
#version 460 core

out float FragColor;

in vec2 TexCoord;

uniform sampler2D ssaoInput;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;

    // 4x4 blur
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoord + offset).r;
        }
    }

    FragColor = result / 16.0;
}
```

---

## Part D: Compose Pass

### [NEW] assets/shaders/compose.frag

```glsl
#version 460 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D gAlbedo;
uniform sampler2D ssaoBlurred;
uniform sampler2D litScene;  // Or compute lighting here

void main() {
    vec3 color = texture(litScene, TexCoord).rgb;
    float ao = texture(ssaoBlurred, TexCoord).r;

    // Apply AO
    FragColor = vec4(color * ao, 1.0);
}
```

---

## Engine Integration

### Render Pipeline Order

```cpp
void Engine::Render() {
    // 1. Geometry pass → G-Buffer
    m_GBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Render all geometry with gbuffer shader
    m_GBuffer->Unbind();

    // 2. SSAO pass → AO texture (half res for performance)
    m_SSAOBuffer->Bind();
    m_SSAOShader->Bind();
    m_GBuffer->BindTextures();
    // Render fullscreen quad
    m_SSAOBuffer->Unbind();

    // 3. SSAO blur pass
    m_SSAOBlurBuffer->Bind();
    m_SSAOBlurShader->Bind();
    // Bind SSAO texture, render fullscreen quad
    m_SSAOBlurBuffer->Unbind();

    // 4. Lighting pass (use G-Buffer + SSAO)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_LightingShader->Bind();
    m_GBuffer->BindTextures();
    m_SSAOBlurBuffer->BindTexture();
    // Render fullscreen quad with deferred lighting
}
```

---

## Validation Checklist

- [ ] G-Buffer renders position/normal/albedo correctly
- [ ] SSAO kernel and noise texture generated
- [ ] SSAO pass produces visible occlusion
- [ ] Blur smooths AO without blurring edges
- [ ] Compose combines AO with lit scene
- [ ] Corners and crevices appear darker
- [ ] Performance: SSAO < 2ms at 1080p
- [ ] Toggle SSAO on/off for comparison

---

## Performance Optimization

| Technique            | Impact                  |
| -------------------- | ----------------------- |
| Half-resolution SSAO | 4x faster               |
| Bilateral blur       | Preserve edges          |
| Reduce kernel size   | 32 samples = 50% faster |
| Temporal filtering   | Smooth over frames      |
