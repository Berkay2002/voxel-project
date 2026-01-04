# Phase 13: Visual & Performance Enhancements

Implementing three features to improve visual quality and rendering performance:
1. **Distance Fog** - Atmospheric effect to fade distant terrain into sky
2. **Block Outline** - Modern outline shader for targeted block highlighting  
3. **Greedy Meshing LOD** - Reduced mesh complexity for distant chunks

---

## User Review Required

> [!IMPORTANT]
> **Greedy Meshing Complexity**: This is a significant algorithm change to the mesh builder. I recommend implementing Phases 13A and 13B first (simpler), then tackling 13C (greedy meshing) as a separate step. Greedy meshing will require careful testing to ensure no visual artifacts.

> [!NOTE]
> **Performance Trade-offs**: Greedy meshing reduces vertex count significantly but increases mesh generation time. This is acceptable since mesh generation is already async, but the algorithm complexity is higher.

---

## Phase 13A: Distance Fog

Distance-based fog that blends terrain into the sky color, hiding chunk boundaries at the edge of render distance.

### Fog Algorithm
Linear interpolation based on fragment distance from camera:
```glsl
float dist = length(v_FragPos - u_CameraPos);
float fogFactor = clamp((u_FogEnd - dist) / (u_FogEnd - u_FogStart), 0.0, 1.0);
vec3 finalColor = mix(u_FogColor, baseColor, fogFactor);
```

### Proposed Changes

#### [MODIFY] [lit.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/lit.vert)
- Add `out vec3 v_FragPos;` (world-space fragment position)
- Pass `v_FragPos = vec3(u_Model * vec4(aPos, 1.0));`

#### [MODIFY] [lit.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/lit.frag)
- Add uniforms: `u_CameraPos`, `u_FogColor`, `u_FogStart`, `u_FogEnd`
- Add `in vec3 v_FragPos;`
- Calculate fog factor and blend with sky color

#### [MODIFY] [water.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/water.vert)
- Add `out vec3 v_FragPos;` matching lit.vert

#### [MODIFY] [water.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/water.frag)
- Add fog calculation matching lit.frag

#### [MODIFY] [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h)
Add fog configuration constants:
```cpp
// Fog settings
constexpr float FOG_START = 80.0f;   // Start fading (blocks)
constexpr float FOG_END = 128.0f;    // Fully fogged (blocks)
// Fog color matches sky: (0.5, 0.7, 1.0)
```

#### [MODIFY] [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp)
- Set fog uniforms in [SetupWorld()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#90-188) and update camera pos each frame in [Render()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#283-328)

---

## Phase 13B: Block Outline Shader

Modern outline rendering for the targeted block, using a slightly scaled-up wireframe cube rendered on top.

### Outline Approach
1. Create a unit cube wireframe mesh (12 edges = 24 vertices as GL_LINES)
2. Scale slightly larger than 1.0 (e.g., 1.005) to sit outside the block
3. Render with depth test enabled but no depth write (always visible but respects occlusion)
4. Use black outline with configurable thickness

### Proposed Changes

#### [NEW] [outline.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/outline.vert)
Simple vertex shader, scales and positions a unit cube at the targeted block position.

```glsl
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_MVP;
uniform vec3 u_BlockPos;
uniform float u_Scale;

void main() {
    vec3 worldPos = u_BlockPos + aPos * u_Scale;
    gl_Position = u_MVP * vec4(worldPos, 1.0);
}
```

#### [NEW] [outline.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/outline.frag)
Simple fragment shader with uniform color.

```glsl
#version 460 core
out vec4 FragColor;
uniform vec4 u_OutlineColor;

void main() {
    FragColor = u_OutlineColor;
}
```

#### [NEW] [BlockOutline.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/BlockOutline.h)
Helper class to manage outline rendering:
- [Setup()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#90-188) - Create VAO/VBO for wireframe cube
- [Render(glm::ivec3 blockPos, Shader& shader, glm::mat4 viewProj)](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#283-328) - Draw outline
- [Cleanup()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#439-449) - Delete GL resources

#### [NEW] [BlockOutline.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/BlockOutline.cpp)
Implementation:
- Define 24 vertices for 12 cube edges (GL_LINES)
- Set `glLineWidth(2.0f)` for visibility

#### [MODIFY] [Engine.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.h)
- Add `std::unique_ptr<Shader> m_OutlineShader;`
- Add `std::unique_ptr<BlockOutline> m_BlockOutline;` (or inline VAO/VBO)

#### [MODIFY] [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp)
- Load outline shader in [SetupWorld()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#90-188)
- Call `BlockOutline::Render()` in [Render()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#283-328) after opaque pass, before water
- Only render if `m_TargetedBlock.hit == true`

#### [MODIFY] [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt)
- Add `core/BlockOutline.cpp` to sources

---

## Phase 13C: Greedy Meshing for LOD

Greedy meshing merges adjacent faces of the same block type into larger quads, dramatically reducing vertex count.

### Algorithm Overview
1. For each face direction (6 faces), sweep through the chunk slice-by-slice
2. Mark each block as "maskable" if it needs a visible face
3. Find rectangles of identical adjacent blocks using greedy expansion
4. Create one large quad instead of many small ones
5. Clear the mask for merged blocks to avoid double-processing

### Proposed Changes

#### [NEW] [GreedyMeshBuilder.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/GreedyMeshBuilder.h)
Greedy meshing implementation:
- [BuildMesh(const Chunk& chunk) -> ChunkMeshResult](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkMeshBuilder.cpp#5-77)
- Internal helpers: `CreateMask()`, `FindRectangle()`, `AddGreedyFace()`

#### [NEW] [GreedyMeshBuilder.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/GreedyMeshBuilder.cpp)
Full greedy meshing algorithm implementation (~300-400 lines).

Key considerations:
- Preserve texture coordinates (scale UVs by quad size)
- Preserve ambient occlusion (use average or skip for LOD)
- Different textures per face prevents merging (grass top vs sides)

#### [MODIFY] [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h)
```cpp
// LOD configuration
constexpr float LOD_DISTANCE_THRESHOLD = 64.0f; // Use greedy mesh beyond this
constexpr bool ENABLE_LOD = true;
```

#### [MODIFY] [ChunkTask.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkTask.h)
- Add LOD mesh data to `ChunkMeshData` struct:
```cpp
std::vector<ChunkVertex> lodOpaqueVertices;
std::vector<unsigned int> lodOpaqueIndices;
// (water LOD optional - water is less common)
```

#### [MODIFY] [Chunk.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/Chunk.h)
- Add LOD GPU resources: `m_LodVAO`, `m_LodVBO`, `m_LodIBO`
- Add `RenderLOD()` method

#### [MODIFY] [Chunk.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/Chunk.cpp)
- Generate LOD mesh data in `GenerateMeshData()` using `GreedyMeshBuilder`
- Upload LOD mesh in `UploadMeshFromData()`

#### [MODIFY] [ChunkManager.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.cpp)
- In `RenderAll()`, calculate distance to chunk center
- Call `chunk->RenderLOD()` instead of `chunk->Render()` when beyond threshold

#### [MODIFY] [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt)
- Add `world/GreedyMeshBuilder.cpp` to sources

---

## Verification Plan

### Automated Tests
```bash
cd /home/berkay-orhan/Developer/playground/voxel-project/build
cmake .. && make -j$(nproc)
./VoxelEngine
```

### Visual Verification

#### Phase 13A (Fog)
- [ ] Distant terrain fades smoothly into sky blue
- [ ] No hard edges at chunk load boundary
- [ ] Water also fades correctly

#### Phase 13B (Block Outline)
- [ ] Black outline appears around targeted block
- [ ] Outline follows crosshair correctly
- [ ] Outline disappears when looking at sky (no target)
- [ ] Outline visible through water but occluded by solid blocks

#### Phase 13C (Greedy Meshing)
- [ ] Distant chunks render with fewer draw calls (check with debug output)
- [ ] No visual artifacts or missing faces on LOD chunks
- [ ] Performance improvement measurable (FPS or frame time)
- [ ] Transition between full mesh and LOD mesh not jarring

---

## File Summary

| Phase | File | Action |
|-------|------|--------|
| 13A | [lit.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/lit.vert), [lit.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/lit.frag) | Modify - add fog |
| 13A | [water.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/water.vert), [water.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/water.frag) | Modify - add fog |
| 13A | [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h) | Modify - fog constants |
| 13A | [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp) | Modify - set fog uniforms |
| 13B | `outline.vert`, `outline.frag` | New shaders |
| 13B | `BlockOutline.h/.cpp` | New helper class |
| 13B | `Engine.h/.cpp` | Modify - outline rendering |
| 13B | [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt) | Modify - add sources |
| 13C | `GreedyMeshBuilder.h/.cpp` | New - greedy meshing |
| 13C | [ChunkTask.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkTask.h), `Chunk.h/.cpp` | Modify - LOD mesh |
| 13C | [ChunkManager.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.cpp) | Modify - LOD selection |
| 13C | [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt) | Modify - add sources |
