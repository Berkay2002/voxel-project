# Phase 6: Lighting Implementation Plan

## Overview

This phase adds visual depth to the voxel engine through:

1. **Directional Lighting** - Sun-like light casting from a fixed direction
2. **Ambient Lighting** - Base illumination to prevent pure black shadows
3. **Per-Vertex Ambient Occlusion (AO)** - Subtle corner/edge darkening

## Current State Analysis

- ✅ `ChunkVertex` already has `normal` field (unused)
- ✅ Face normals are calculated in `ChunkMeshBuilder::GetFaceNormal()`
- ❌ Shaders don't use normals for lighting
- ❌ No ambient occlusion calculation

---

## Part A: Directional + Ambient Lighting

### Step 1: Create Lit Shaders

**File: `assets/shaders/lit.vert`**

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;

uniform mat4 u_MVP;
uniform mat4 u_Model;  // For transforming normals

void main() {
    gl_Position = u_MVP * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    // Transform normal to world space (for static chunks, this is just the normal)
    Normal = mat3(u_Model) * aNormal;
}
```

**File: `assets/shaders/lit.frag`**

```glsl
#version 460 core

in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3 u_LightDir;        // Normalized direction TO the light
uniform float u_AmbientStrength;  // 0.0 - 1.0 (recommend 0.3-0.4)

void main() {
    vec4 texColor = texture(u_Texture, TexCoord);

    // Diffuse lighting (Lambertian)
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, u_LightDir), 0.0);

    // Combine ambient + diffuse
    float lighting = u_AmbientStrength + (1.0 - u_AmbientStrength) * diff;

    FragColor = vec4(texColor.rgb * lighting, texColor.a);
}
```

### Step 2: Update Vertex Attribute Layout

The current `ChunkVertex` struct is:

```cpp
struct ChunkVertex {
    glm::vec3 position;  // location 0
    glm::vec2 uv;        // location 1
    glm::vec3 normal;    // location 2 (NEW - needs enabling)
};
```

**Update `Chunk::UploadMesh()` or `UploadMeshFromData()`** to set up the normal attribute:

```cpp
// After existing position (0) and uv (1) attributes:
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex),
                      (void*)offsetof(ChunkVertex, normal));
```

### Step 3: Engine Integration

**In `Engine.cpp`**:

```cpp
// Instead of m_TexturedShader, load the lit shader
m_LitShader = std::make_unique<Shader>("assets/shaders/lit.vert",
                                        "assets/shaders/lit.frag");

// In Render():
m_LitShader->Use();
m_LitShader->SetVec3("u_LightDir", glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));  // Sun angle
m_LitShader->SetFloat("u_AmbientStrength", 0.35f);
// ... existing MVP and texture uniforms
```

---

## Part B: Per-Vertex Ambient Occlusion

AO makes blocks look more grounded by darkening vertices that are "occluded" by neighboring blocks.

### Step 4: Add AO to ChunkVertex

**Update `ChunkMeshBuilder.h`**:

```cpp
struct ChunkVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    float ao;  // 0.0 (fully occluded) to 1.0 (fully lit)
};
```

### Step 5: Calculate AO in ChunkMeshBuilder

For each vertex of a face, check 3 neighboring positions:

- The 2 edge-adjacent blocks
- The 1 corner-adjacent block

**AO Formula** (standard Minecraft-style):

```cpp
float CalculateAO(bool side1, bool side2, bool corner) {
    if (side1 && side2) return 0.0f;  // Fully occluded
    return (3.0f - (side1 + side2 + corner)) / 3.0f;
}
```

This gives values: 1.0, 0.67, 0.33, 0.0 depending on occlusion.

### Step 6: Pass AO to Shader

**Update `lit.vert`**:

```glsl
layout (location = 3) in float aAO;
out float AO;
// ...
AO = aAO;
```

**Update `lit.frag`**:

```glsl
in float AO;
// ...
float lighting = (u_AmbientStrength + (1.0 - u_AmbientStrength) * diff) * AO;
```

### Step 7: Update Vertex Attribute Layout for AO

```cpp
glEnableVertexAttribArray(3);
glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex),
                      (void*)offsetof(ChunkVertex, ao));
```

---

## Files to Create/Modify

| Action | File                                                  |
| ------ | ----------------------------------------------------- |
| CREATE | `assets/shaders/lit.vert`                             |
| CREATE | `assets/shaders/lit.frag`                             |
| MODIFY | `world/ChunkMeshBuilder.h` (add `ao` field)           |
| MODIFY | `world/ChunkMeshBuilder.cpp` (calculate AO)           |
| MODIFY | `world/Chunk.cpp` (enable normal + AO vertex attribs) |
| MODIFY | `core/Engine.cpp` (load lit shader, set uniforms)     |

---

## Verification Plan

1. **Build Test**: `cmake --build build`
2. **Visual Check**:
   - Top faces of blocks should be brightest
   - Side faces at different angles show varying brightness
   - Corners/edges between blocks show subtle darkening
3. **Debug Tips**:
   - Set `u_AmbientStrength = 1.0` → All faces same brightness (confirms AO only)
   - Set `u_LightDir = (0, 1, 0)` → Only top faces lit (confirms normals)

---

## Notes

- **Performance**: AO calculation adds minimal overhead (only during mesh generation, not per-frame)
- **Optional Enhancement**: Smooth AO by interpolating across vertices (currently per-vertex, so naturally interpolated by GPU)
- **Future**: This lighting system can later be extended with sunlight propagation for caves/overhangs
