# Phase 17: Voxel Light Propagation Implementation Plan

## Overview

Implement Minecraft-style lighting with two light channels:

- **Skylight** (0-15): Propagates from sky downward, blocked by solid blocks
- **Block Light** (0-15): Emitted by torches, glowstone, lava

Both channels use **flood-fill BFS** for propagation.

---

## Architecture

```
Terrain Generation
       ↓
Chunk Ready → Skylight Propagation (top-down flood fill)
       ↓
Block Placed → Block Light Propagation (BFS from emitter)
       ↓
Mesh Building → Sample light levels per vertex
       ↓
Shader → Apply light level as brightness multiplier
```

---

## Part A: Light Map Storage

### [NEW] world/LightMap.h

```cpp
#pragma once
#include <array>
#include <cstdint>

namespace Voxel {

// Light levels 0-15, stored as nibbles (4 bits each)
// Upper nibble = skylight, lower nibble = block light
class LightMap {
public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_HEIGHT = 256;

    LightMap() = default;

    // Skylight (0-15)
    [[nodiscard]] uint8_t GetSkyLight(int x, int y, int z) const;
    void SetSkyLight(int x, int y, int z, uint8_t level);

    // Block light (0-15)
    [[nodiscard]] uint8_t GetBlockLight(int x, int y, int z) const;
    void SetBlockLight(int x, int y, int z, uint8_t level);

    // Combined light = max(skylight * skyMultiplier, blockLight)
    [[nodiscard]] float GetCombinedLight(int x, int y, int z, float skyMultiplier = 1.0f) const;

    // Initialize all to 0
    void Clear();

    // Fill skylight from top (call after terrain generation)
    void InitializeSkylight(const class Chunk& chunk);

private:
    // Packed storage: upper 4 bits = sky, lower 4 bits = block
    std::array<uint8_t, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_Data{};

    [[nodiscard]] size_t GetIndex(int x, int y, int z) const {
        return static_cast<size_t>(y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x);
    }
};

} // namespace Voxel
```

---

### [NEW] world/LightMap.cpp

```cpp
#include "world/LightMap.h"
#include "world/Chunk.h"
#include "world/Block.h"
#include <queue>

namespace Voxel {

uint8_t LightMap::GetSkyLight(int x, int y, int z) const {
    return (m_Data[GetIndex(x, y, z)] >> 4) & 0x0F;
}

void LightMap::SetSkyLight(int x, int y, int z, uint8_t level) {
    size_t idx = GetIndex(x, y, z);
    m_Data[idx] = (m_Data[idx] & 0x0F) | ((level & 0x0F) << 4);
}

uint8_t LightMap::GetBlockLight(int x, int y, int z) const {
    return m_Data[GetIndex(x, y, z)] & 0x0F;
}

void LightMap::SetBlockLight(int x, int y, int z, uint8_t level) {
    size_t idx = GetIndex(x, y, z);
    m_Data[idx] = (m_Data[idx] & 0xF0) | (level & 0x0F);
}

float LightMap::GetCombinedLight(int x, int y, int z, float skyMultiplier) const {
    uint8_t sky = GetSkyLight(x, y, z);
    uint8_t block = GetBlockLight(x, y, z);
    float skyBrightness = (sky / 15.0f) * skyMultiplier;
    float blockBrightness = block / 15.0f;
    return std::max(skyBrightness, blockBrightness);
}

void LightMap::Clear() {
    std::fill(m_Data.begin(), m_Data.end(), 0);
}

void LightMap::InitializeSkylight(const Chunk& chunk) {
    // Phase 1: Direct skylight (straight down from sky)
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            uint8_t lightLevel = 15;

            // Trace from top down
            for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                BlockID block = chunk.GetBlock(x, y, z);

                if (block == BLOCK_AIR) {
                    SetSkyLight(x, y, z, lightLevel);
                } else if (IsTransparent(block)) {
                    // Transparent blocks (water, glass) reduce light
                    lightLevel = std::max(0, lightLevel - 1);
                    SetSkyLight(x, y, z, lightLevel);
                } else {
                    // Solid block - no skylight below
                    lightLevel = 0;
                    SetSkyLight(x, y, z, 0);
                }
            }
        }
    }

    // Phase 2: Horizontal spread (BFS)
    // This allows light to spread into caves from entrances
    PropagateSkylightBFS(chunk);
}

void LightMap::PropagateSkylightBFS(const Chunk& chunk) {
    struct LightNode {
        int x, y, z;
        uint8_t level;
    };

    std::queue<LightNode> queue;

    // Seed queue with all blocks that have skylight
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                uint8_t light = GetSkyLight(x, y, z);
                if (light > 1) {
                    queue.push({x, y, z, light});
                }
            }
        }
    }

    // Offsets for 6 neighbors
    constexpr int dx[] = {1, -1, 0, 0, 0, 0};
    constexpr int dy[] = {0, 0, 1, -1, 0, 0};
    constexpr int dz[] = {0, 0, 0, 0, 1, -1};

    while (!queue.empty()) {
        LightNode node = queue.front();
        queue.pop();

        for (int i = 0; i < 6; ++i) {
            int nx = node.x + dx[i];
            int ny = node.y + dy[i];
            int nz = node.z + dz[i];

            // Bounds check
            if (nx < 0 || nx >= CHUNK_SIZE ||
                ny < 0 || ny >= CHUNK_HEIGHT ||
                nz < 0 || nz >= CHUNK_SIZE) {
                continue;
            }

            // Skip solid blocks
            BlockID block = chunk.GetBlock(nx, ny, nz);
            if (IsOpaque(block)) continue;

            // Propagate light (decrease by 1)
            uint8_t newLevel = node.level - 1;
            if (newLevel > GetSkyLight(nx, ny, nz)) {
                SetSkyLight(nx, ny, nz, newLevel);
                if (newLevel > 1) {
                    queue.push({nx, ny, nz, newLevel});
                }
            }
        }
    }
}

} // namespace Voxel
```

---

## Part B: Block Light Propagation

### Add to blocks.json

```json
{
  "name": "torch",
  "id": 20,
  "lightLevel": 14,
  "textures": {
    "all": "torch"
  }
}
```

### Light Propagation on Block Place

```cpp
void ChunkManager::PropagateBlockLight(int wx, int wy, int wz, uint8_t level) {
    struct LightNode { int x, y, z; uint8_t level; };
    std::queue<LightNode> queue;
    queue.push({wx, wy, wz, level});

    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        // Get chunk and local coords
        auto [chunk, lx, ly, lz] = GetChunkAndLocal(node.x, node.y, node.z);
        if (!chunk) continue;

        // Set light level
        chunk->GetLightMap().SetBlockLight(lx, ly, lz, node.level);

        // Propagate to neighbors
        if (node.level <= 1) continue;

        for (auto [dx, dy, dz] : neighbors) {
            // ... similar BFS with level - 1
        }
    }
}
```

---

## Part C: Mesh Builder Integration

### [MODIFY] world/ChunkVertex.h

```cpp
struct ChunkVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    float ao;
    float texIndex;
    glm::vec3 tintColor;
    float lightLevel;  // NEW: 0.0 - 1.0 combined light
};
```

### [MODIFY] world/ChunkMeshBuilder.cpp

When adding vertices, sample light from LightMap:

```cpp
void ChunkMeshBuilder::AddFace(..., const LightMap& lightMap, float skyMultiplier) {
    // Get light level at vertex position (average of adjacent blocks)
    float light = lightMap.GetCombinedLight(x, y, z, skyMultiplier);

    // Add to vertex
    vertex.lightLevel = light;
}
```

---

## Part D: Shader Integration

### [MODIFY] lit.vert

```glsl
layout (location = 6) in float aLightLevel;

out float LightLevel;

void main() {
    // ... existing code
    LightLevel = aLightLevel;
}
```

### [MODIFY] lit.frag

```glsl
in float LightLevel;

void main() {
    // ... existing lighting calculation

    // Apply voxel light level (minimum 0.05 for visibility)
    float voxelLight = max(LightLevel, 0.05);
    vec3 litColor = tintedColor * lighting * voxelLight;

    // ... fog calculation
}
```

---

## Part E: Chunk System Updates

### [MODIFY] Chunk.h

```cpp
class Chunk {
    // ... existing members

    LightMap m_LightMap;

public:
    LightMap& GetLightMap() { return m_LightMap; }
    const LightMap& GetLightMap() const { return m_LightMap; }
};
```

### [MODIFY] TerrainGenerator.cpp

After generating terrain, initialize skylight:

```cpp
void TerrainGenerator::Generate(Chunk& chunk, ...) {
    // ... existing terrain generation

    // Initialize skylight after terrain is complete
    chunk.GetLightMap().InitializeSkylight(chunk);
}
```

---

## Validation Checklist

- [ ] LightMap compiles and stores light data correctly
- [ ] Skylight propagates from top of world
- [ ] Underground caves are dark (light level 0)
- [ ] Cave entrances show light gradient
- [ ] Torch blocks emit light level 14
- [ ] Block light propagates through air
- [ ] Light stops at solid blocks
- [ ] Cross-chunk light propagation works
- [ ] Vertex attribute uploaded correctly
- [ ] Shader applies light level
- [ ] Night reduces skylight (skyMultiplier < 1)
- [ ] Performance: light propagation < 10ms per chunk

---

## Performance Considerations

| Technique               | Impact                    |
| ----------------------- | ------------------------- |
| BFS with max 15 depth   | Bounded propagation       |
| Nibble packing (4 bits) | 50% memory savings        |
| Incremental updates     | Only recalc changed areas |
| Async propagation       | Off main thread           |
