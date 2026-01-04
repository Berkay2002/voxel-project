# Phase 3A: Chunk Data & Single Chunk Rendering

## Goal

Implement the core voxel chunk system: block types, chunk data storage (16×16×256), mesh generation with **face culling**, and render a single procedural chunk.

---

## New Files

### world/Block.h

- `BlockType` enum: `Air`, `Dirt`, `Grass`, `Stone`
- `IsOpaque(BlockType)` helper
- `GetBlockUV(BlockType, Face)` for texture mapping

### world/Chunk.h/.cpp

- Constants: `CHUNK_WIDTH=16`, `CHUNK_HEIGHT=256`, `CHUNK_DEPTH=16`
- 1D array: `BlockType m_Blocks[16*256*16]`
- `GetBlock(x, y, z)` / `SetBlock(x, y, z, type)`
- Index: `(y * CHUNK_WIDTH * CHUNK_DEPTH) + (z * CHUNK_WIDTH) + x`

### world/ChunkMeshBuilder.h/.cpp

- `BuildMesh(const Chunk&)` → vertex/index data
- Face culling: skip face if neighbor is opaque
- Vertex format: `{vec3 pos, vec2 uv, vec3 normal}`

---

## Modified Files

### CMakeLists.txt

- Add `world/*.cpp` to sources

### core/Engine.cpp

- Create test `Chunk` with height-based terrain
- Build mesh with `ChunkMeshBuilder`
- Render with textured shader

---

## Face Culling Logic

```cpp
for each block in chunk:
    if block == Air: skip
    for each of 6 faces:
        neighbor = GetNeighborBlock(direction)
        if neighbor is Air or out-of-bounds:
            AddFaceVertices(pos, face, type)
```

---

## Validation

```bash
cd build && cmake .. && make -j$(nproc) && ./VoxelEngine
```

- Single chunk renders with terrain
- Internal faces NOT visible
- Grass top, dirt below, stone at bottom
