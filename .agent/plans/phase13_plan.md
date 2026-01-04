# Phase 13: Visual & Performance Enhancements

## Status Summary

| Phase   | Feature            | Status      |
| ------- | ------------------ | ----------- |
| **13A** | Distance Fog       | ✅ Complete |
| **13B** | Block Outline      | ✅ Complete |
| **13C** | Greedy Meshing LOD | 🔜 Backlog  |

---

## Phase 13A: Distance Fog ✅

**Completed 2026-01-04**

Linear distance-based fog that blends terrain into sky color.

### Files Modified

- `lit.vert` - Added `FragWorldPos` output
- `lit.frag` - Added fog uniforms and calculation
- `water.vert` - Added `FragWorldPos` output
- `water.frag` - Added fog calculation + alpha fade
- `WorldConfig.h` - Added `FOG_START`, `FOG_END` constants
- `Engine.cpp` - Set fog uniforms, update camera pos each frame

---

## Phase 13B: Block Outline ✅

**Completed 2026-01-04**

Wireframe outline around targeted block (Minecraft-style).

### Files Created

- `outline.vert` - Outline vertex shader
- `outline.frag` - Outline fragment shader
- `BlockOutline.h` - Helper class header
- `BlockOutline.cpp` - Wireframe cube VAO/VBO + rendering

### Files Modified

- `Engine.h` - Added outline shader + BlockOutline members
- `Engine.cpp` - Initialize and render outline
- `CMakeLists.txt` - Added BlockOutline.cpp

---

## Phase 13C: Greedy Meshing LOD (Backlog)

Reduces vertex count for distant chunks by merging adjacent same-type faces into larger quads.

### Algorithm

1. Sweep each face direction slice-by-slice
2. Build visibility mask for visible faces
3. Greedy-expand rectangles of identical block types
4. Generate single large quad per merged region

### Files to Create

- `GreedyMeshBuilder.h/.cpp` - Greedy meshing algorithm

### Files to Modify

- `WorldConfig.h` - LOD distance threshold
- `ChunkTask.h` - LOD mesh data
- `Chunk.h/.cpp` - LOD VAO/VBO + `RenderLOD()`
- `ChunkManager.cpp` - Distance-based mesh selection
- `CMakeLists.txt` - Add sources
