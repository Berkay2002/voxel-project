# Future: RTX Migration Research Notes

## Overview

This document outlines the research and planning for migrating the Voxel Engine from pure OpenGL rasterization to hybrid or full ray tracing using NVIDIA RTX hardware.

---

## Why Ray Tracing?

| Feature                 | Rasterization                             | Ray Tracing                           |
| ----------------------- | ----------------------------------------- | ------------------------------------- |
| **Shadows**             | Shadow maps (artifacts, limited cascades) | Perfect, soft shadows at any distance |
| **Reflections**         | Screen-space (can't reflect off-screen)   | True reflections of entire world      |
| **Global Illumination** | Baked lightmaps / voxel cone tracing      | Path-traced GI (color bleeding)       |
| **Ambient Occlusion**   | SSAO (screen-space only)                  | Ray-traced AO (accurate)              |
| **Transparency**        | Order-dependent, complex sorting          | Correct refraction/reflection         |

---

## Hardware Requirements

- **GPU**: NVIDIA RTX 20-series or newer (RTX 3090 ✓)
- **Driver**: 460.89+ for Vulkan RT extensions
- **VRAM**: 8GB+ recommended for BVH structures

---

## Migration Paths

### Option A: Vulkan + VK_KHR_ray_tracing_pipeline

**Architecture:**

```
Vulkan Instance
    ↓
Physical Device (RTX GPU)
    ↓
Logical Device with RT extensions
    ↓
Acceleration Structures (BLAS/TLAS)
    ↓
Ray Tracing Pipeline
    ↓
Shader Binding Table (raygen, miss, closesthit)
```

**Pros:**

- Cross-platform (Windows, Linux)
- Full control over pipeline
- Can mix rasterization + RT
- Industry standard

**Cons:**

- **MAJOR engine rewrite** (6000+ lines)
- Complex synchronization (semaphores, barriers)
- Verbose API (500+ functions)
- No OpenGL interop (must port all rendering)

**Estimated Effort:** 2-3 months full-time

---

### Option B: NVIDIA OptiX 7+

**Architecture:**

```
CUDA Context
    ↓
OptiX Device Context
    ↓
Module (PTX shaders)
    ↓
Acceleration Structures
    ↓
Pipeline + SBT
    ↓
optixLaunch()
```

**Pros:**

- High-level RT API
- AI denoising built-in (OptiX Denoiser)
- Excellent for pure RT applications
- Good documentation

**Cons:**

- **NVIDIA-only** (no AMD support)
- Separate from rasterizer (need interop)
- CUDA dependency

**Estimated Effort:** 1-2 months

---

### Option C: Hybrid OpenGL + Compute Shader RT

**Architecture:**

```
OpenGL (existing)
    ↓
Compute Shader (software ray tracing)
    ↓
Output to texture
    ↓
Composite in fragment shader
```

**Pros:**

- Keep existing OpenGL code
- Gradual migration
- No new dependencies

**Cons:**

- **No hardware RT acceleration** (very slow)
- Limited to simple effects
- Performance: 10-100x slower than HW RT

**Estimated Effort:** 2-4 weeks

---

## Recommended Hybrid Approach

Given the project's current OpenGL foundation, I recommend:

### Phase 1: Keep OpenGL for Rasterization

- Continue using OpenGL for terrain, water, UI
- Benefit from existing optimizations

### Phase 2: Add Vulkan for RT Features Only

- Create Vulkan context alongside OpenGL
- Use VK_EXT_external_memory for interop
- Render RT effects to shared texture

### Phase 3: Start with RT Shadows

- Biggest visual impact
- Simpler than full GI
- Single ray per pixel

### Phase 4: Add RT Reflections for Water

- Currently water is flat colored
- RT can provide true world reflections
- Multi-bounce for underwater caustics

### Phase 5: Evaluate Full Migration

- After RT features working, assess performance
- Consider full Vulkan migration if beneficial

---

## Vulkan RT Implementation Outline

### Step 1: Vulkan Instance + Device Setup

```cpp
VkInstance instance;
VkPhysicalDevice physicalDevice;
VkDevice device;

// Required extensions
std::vector<const char*> deviceExtensions = {
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
};
```

### Step 2: Acceleration Structure

**BLAS (Bottom-Level):** One per chunk mesh

```cpp
VkAccelerationStructureGeometryKHR geometry{};
geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
geometry.geometry.triangles.vertexData = vertexBuffer;
geometry.geometry.triangles.indexData = indexBuffer;
```

**TLAS (Top-Level):** References all chunk BLAS

```cpp
VkAccelerationStructureInstanceKHR instance{};
instance.accelerationStructureReference = blasAddress;
instance.transform = chunkTransform;
```

### Step 3: Ray Tracing Shaders

**Ray Generation (raygen.rgen):**

```glsl
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadEXT vec3 hitValue;

void main() {
    vec2 uv = (gl_LaunchIDEXT.xy + 0.5) / gl_LaunchSizeEXT.xy;

    // Calculate ray direction from camera
    vec3 origin = cameraPos;
    vec3 direction = calculateRayDir(uv);

    traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xFF,
                0, 0, 0, origin, 0.001, direction, 10000.0, 0);

    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 1.0));
}
```

**Closest Hit (closesthit.rchit):**

```glsl
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main() {
    // Get hit information
    vec3 worldPos = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;

    // Simple diffuse color (or sample from voxel data)
    hitValue = vec3(0.5);
}
```

**Miss (miss.rmiss):**

```glsl
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main() {
    // Sky color
    hitValue = vec3(0.5, 0.7, 1.0);
}
```

---

## Voxel-Specific Optimizations

Voxel worlds are **ideal for ray tracing** due to:

1. **Axis-aligned geometry** → Fast BVH traversal
2. **Regular grid structure** → DDA can accelerate traversal
3. **Existing DDA code** → Already have voxel raycast for block picking
4. **Low poly count per chunk** → Efficient BLAS

### Hybrid DDA + BVH Approach

```
TLAS (chunk level)
    ↓
DDA in chunk space (voxel grid)
    ↓
Per-voxel material/color
```

This can be **faster than triangle-mesh RT** for dense voxel worlds.

---

## AI Denoising

For path-traced GI (noisy at low sample counts), use:

- **OptiX AI Denoiser** — NVIDIA's trained neural network
- **NVIDIA Real-Time Denoisers (NRD)** — Open-source, Vulkan-compatible

Typical pipeline:

```
1 sample per pixel (noisy) → Denoiser → Clean image
```

---

## Research Resources

### Documentation

- [Vulkan Ray Tracing Tutorial](https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/)
- [OptiX 7 Programming Guide](https://raytracing-docs.nvidia.com/optix7/guide/)
- [Vulkan Spec - Ray Tracing](https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#ray-tracing)

### Example Projects

- [vk_raytrace](https://github.com/nvpro-samples/vk_raytrace) - NVIDIA Vulkan RT samples
- [Quake II RTX](https://github.com/NVIDIA/Q2RTX) - Vulkan RT game implementation

### Voxel-Specific RT

- [GVDB-Voxels](https://github.com/NVIDIA/gvdb-voxels) - NVIDIA sparse voxel database
- [John Lin's Voxel RT](https://jacco.ompf2.com/2021/03/22/voxel-ray-tracing/) - Blog series

---

## Timeline Estimate

| Phase | Task                               | Duration  |
| ----- | ---------------------------------- | --------- |
| 1     | Research + prototype (Vulkan init) | 1-2 weeks |
| 2     | BLAS/TLAS for chunks               | 1 week    |
| 3     | Basic RT shadows                   | 2 weeks   |
| 4     | OpenGL interop                     | 1 week    |
| 5     | RT reflections for water           | 1 week    |
| 6     | Performance optimization           | 1-2 weeks |
| 7     | (Optional) Full GI                 | 2-4 weeks |

**Total:** 2-3 months for basic hybrid RT
