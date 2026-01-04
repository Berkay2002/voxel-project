# Phase 15: Shadow Mapping Implementation Plan

## Overview

Render the scene from the sun's perspective into a depth buffer (shadow map), then sample it during normal rendering to determine if a fragment is in shadow.

## Architecture

```
Sun Position → Light-Space Matrix → Shadow Pass → Shadow Map Texture
                                                        ↓
Main Pass → Fragment Shader → Sample Shadow Map → Apply Shadow Factor
```

---

## Part A: Shadow Map Infrastructure

### [NEW] core/ShadowMap.h/.cpp

```cpp
class ShadowMap {
public:
    bool Create(int width, int height);
    void Bind();      // Bind FBO for shadow pass
    void Unbind();    // Restore default framebuffer
    void BindTexture(int slot);  // Bind depth texture for sampling

    [[nodiscard]] int GetWidth() const { return m_Width; }
    [[nodiscard]] int GetHeight() const { return m_Height; }

private:
    unsigned int m_FBO = 0;
    unsigned int m_DepthTexture = 0;
    int m_Width = 0;
    int m_Height = 0;
};
```

**Key Implementation Details:**

- Use `GL_DEPTH_COMPONENT32F` for depth texture
- Set `GL_TEXTURE_COMPARE_MODE` to `GL_COMPARE_REF_TO_TEXTURE`
- Border color = 1.0 (no shadow outside map)
- No color attachment (depth-only)

---

### [NEW] assets/shaders/shadow.vert

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;

void main() {
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(aPos, 1.0);
}
```

### [NEW] assets/shaders/shadow.frag

```glsl
#version 460 core

void main() {
    // Empty - only depth is written
}
```

---

## Part B: Light-Space Matrix Calculation

Add to `SkyRenderer` or `Engine`:

```cpp
glm::mat4 CalculateLightSpaceMatrix(const glm::vec3& sunDir,
                                     const glm::vec3& cameraPos,
                                     float shadowDistance) {
    // Light position far from scene
    glm::vec3 lightPos = cameraPos - sunDir * shadowDistance;

    // Orthographic projection for directional light
    float orthoSize = shadowDistance;
    glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize,
                                      -orthoSize, orthoSize,
                                      1.0f, shadowDistance * 2.0f);

    // Look from light position toward scene center
    glm::mat4 lightView = glm::lookAt(lightPos, cameraPos, glm::vec3(0, 1, 0));

    return lightProj * lightView;
}
```

---

## Part C: Shader Integration

### [MODIFY] assets/shaders/lit.frag

Add uniforms:

```glsl
uniform sampler2DShadow u_ShadowMap;
uniform mat4 u_LightSpaceMatrix;
```

Add shadow calculation function:

```glsl
float CalculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;  // Transform to [0,1]

    // Outside shadow map = no shadow
    if (projCoords.z > 1.0) return 0.0;

    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec3 samplePos = vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z);
            shadow += texture(u_ShadowMap, samplePos);
        }
    }
    shadow /= 9.0;

    // Bias to prevent shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

    return shadow;
}
```

Modify main():

```glsl
// Transform fragment to light space
vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(FragWorldPos, 1.0);
float shadow = CalculateShadow(fragPosLightSpace, norm, u_LightDir);

// Apply shadow to diffuse (not ambient)
float lighting = u_AmbientStrength + (1.0 - u_AmbientStrength) * diff * shadow;
lighting *= AO;
```

---

## Part D: Engine Integration

### [MODIFY] Engine.h

Add members:

```cpp
std::unique_ptr<ShadowMap> m_ShadowMap;
std::unique_ptr<Shader> m_ShadowShader;
glm::mat4 m_LightSpaceMatrix;
```

### [MODIFY] Engine.cpp

**Setup:**

```cpp
// In SetupWorld()
m_ShadowMap = std::make_unique<ShadowMap>();
m_ShadowMap->Create(2048, 2048);

m_ShadowShader = std::make_unique<Shader>("assets/shaders/shadow.vert",
                                           "assets/shaders/shadow.frag");
```

**Render loop:**

```cpp
// === SHADOW PASS ===
m_LightSpaceMatrix = CalculateLightSpaceMatrix(
    m_SkyRenderer->GetSunDirection(),
    m_Camera->GetPosition(),
    128.0f  // Shadow distance
);

m_ShadowMap->Bind();
glClear(GL_DEPTH_BUFFER_BIT);
glViewport(0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight());

m_ShadowShader->Bind();
m_ShadowShader->SetMat4("u_LightSpaceMatrix", m_LightSpaceMatrix);
m_ChunkManager->RenderAllShadow(*m_ShadowShader, m_LightSpaceMatrix);
m_ShadowShader->Unbind();

m_ShadowMap->Unbind();
glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

// === MAIN PASS ===
m_ShadowMap->BindTexture(1);  // Slot 1 for shadow map
m_Shader->Bind();
m_Shader->SetMat4("u_LightSpaceMatrix", m_LightSpaceMatrix);
m_Shader->SetInt("u_ShadowMap", 1);
// ... rest of rendering
```

---

## Part E: Cascaded Shadow Maps (Optional)

For large view distances (320+ blocks), use 3 cascades:

| Cascade | Distance       | Resolution | Coverage    |
| ------- | -------------- | ---------- | ----------- |
| 0       | 0-32 blocks    | 2048×2048  | Near detail |
| 1       | 32-128 blocks  | 2048×2048  | Mid range   |
| 2       | 128-320 blocks | 1024×1024  | Far shadows |

**Implementation:**

1. Create 3 `ShadowMap` objects
2. Calculate 3 light-space matrices with different ortho sizes
3. Pass all 3 shadow maps to fragment shader
4. Select cascade based on fragment's view-space depth

---

## Validation Checklist

- [ ] Shadow map FBO created without errors
- [ ] Shadow pass renders depth correctly
- [ ] Main pass samples shadow map
- [ ] Shadows appear on terrain from sun direction
- [ ] No shadow acne (floating shadows on surfaces)
- [ ] No peter-panning (shadows disconnected from objects)
- [ ] Performance: shadow pass < 5ms
- [ ] Shadows transition smoothly at dawn/dusk

---

## Performance Considerations

| Setting               | Impact                | Recommendation         |
| --------------------- | --------------------- | ---------------------- |
| Shadow map resolution | Quality vs VRAM       | 2048×2048 for RTX 3090 |
| PCF kernel size       | Softness vs speed     | 3×3 (9 samples)        |
| Shadow distance       | Coverage vs quality   | 128-256 blocks         |
| Update frequency      | Every frame vs cached | Every frame            |
