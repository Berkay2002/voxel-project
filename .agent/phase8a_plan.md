# Phase 8A: Cave Generation (Spaghetti Caves)

## Goal

Implement Minecraft-style **spaghetti caves** using 3D Perlin noise, with a modular architecture that allows easy addition of **cheese caves** and **noodle caves** in future phases.

---

## Design Overview

### Modular Cave System Architecture

To support multiple cave types in the future, we'll introduce a **cave carver abstraction**:

```
┌─────────────────────────────────────────────────────┐
│                  TerrainGenerator                   │
├─────────────────────────────────────────────────────┤
│  1. Generate terrain heights (existing)             │
│  2. Fill columns with Stone/Dirt/Grass              │
│  3. Apply CaveCarvers (modular)                     │
│  4. Fill underwater caves with Water                │
└─────────────────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────┐
│                   ICaveCarver                       │
│  (Abstract interface for cave generation)           │
├─────────────────────────────────────────────────────┤
│  + Configure(seed)                                  │
│  + ShouldCarve(worldX, worldY, worldZ) → bool       │
└─────────────────────────────────────────────────────┘
          ▲              ▲              ▲
          │              │              │
    ┌─────┴────┐   ┌────┴─────┐   ┌────┴─────┐
    │ Spaghetti │   │  Cheese  │   │  Noodle  │
    │  Carver   │   │  Carver  │   │  Carver  │
    │ (Phase 8A)│   │ (Future) │   │ (Future) │
    └──────────┘   └──────────┘   └──────────┘
```

---

## Spaghetti Cave Parameters (Minecraft-Like)

| Parameter           | Value | Description                            |
| ------------------- | ----- | -------------------------------------- |
| `minY`              | 5     | Lowest cave level (above "bedrock")    |
| `maxY`              | 52    | Highest cave level (underground)       |
| `surfaceProtection` | 4     | Don't carve within N blocks of surface |
| `frequency`         | 0.05  | Noise frequency (medium tunnels)       |
| `threshold`         | 0.5   | Carve when `abs(noise) < threshold`    |
| `ySquash`           | 1.5   | Stretch caves horizontally             |

The `abs(noise) < threshold` technique creates tube-like tunnels (values near 0 form connected passages).

---

## Proposed Changes

### New Files

#### [NEW] [ICaveCarver.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ICaveCarver.h)

Abstract interface for cave carvers:

```cpp
#pragma once
#include <cstdint>

namespace Voxel {

class ICaveCarver {
public:
    virtual ~ICaveCarver() = default;

    // Configure the carver with a seed
    virtual void Configure(int seed) = 0;

    // Check if a block at world position should be carved
    virtual bool ShouldCarve(int worldX, int worldY, int worldZ,
                             int terrainHeight, int seaLevel) const = 0;

    // Get display name for debugging
    virtual const char* GetName() const = 0;
};

} // namespace Voxel
```

---

#### [NEW] [SpaghettiCaveCarver.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SpaghettiCaveCarver.h)

Configuration and header for spaghetti caves:

```cpp
#pragma once
#include "ICaveCarver.h"

namespace Voxel {

struct SpaghettiCaveConfig {
    int minY = 5;               // Lowest cave level
    int maxY = 52;              // Highest cave level
    int surfaceProtection = 4;  // Blocks below surface to protect
    float frequency = 0.05f;    // Noise frequency
    float threshold = 0.5f;     // Carve threshold (abs(noise) < threshold)
    float ySquash = 1.5f;       // Horizontal stretching factor
};

class SpaghettiCaveCarver : public ICaveCarver {
public:
    SpaghettiCaveCarver();
    explicit SpaghettiCaveCarver(const SpaghettiCaveConfig& config);

    void Configure(int seed) override;
    bool ShouldCarve(int worldX, int worldY, int worldZ,
                     int terrainHeight, int seaLevel) const override;
    const char* GetName() const override { return "SpaghettiCaves"; }

    void SetConfig(const SpaghettiCaveConfig& config);

private:
    SpaghettiCaveConfig m_Config;
    // Note: FastNoiseLite instance in .cpp as static or member
};

} // namespace Voxel
```

---

#### [NEW] [SpaghettiCaveCarver.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SpaghettiCaveCarver.cpp)

Implementation of spaghetti cave carving:

```cpp
#include "SpaghettiCaveCarver.h"
#include "FastNoiseLite.h"
#include <cmath>

namespace Voxel {

static FastNoiseLite s_CaveNoise;

SpaghettiCaveCarver::SpaghettiCaveCarver() {
    SetConfig(SpaghettiCaveConfig{});
}

SpaghettiCaveCarver::SpaghettiCaveCarver(const SpaghettiCaveConfig& config) {
    SetConfig(config);
}

void SpaghettiCaveCarver::Configure(int seed) {
    s_CaveNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_CaveNoise.SetSeed(seed + 1000); // Offset from terrain seed
    s_CaveNoise.SetFrequency(m_Config.frequency);
}

void SpaghettiCaveCarver::SetConfig(const SpaghettiCaveConfig& config) {
    m_Config = config;
}

bool SpaghettiCaveCarver::ShouldCarve(int worldX, int worldY, int worldZ,
                                       int terrainHeight, int seaLevel) const {
    // Don't carve outside Y range
    if (worldY < m_Config.minY || worldY > m_Config.maxY) {
        return false;
    }

    // Don't carve near surface (protect top layers)
    if (worldY >= terrainHeight - m_Config.surfaceProtection) {
        return false;
    }

    // Don't carve if terrain is underwater (avoid carving ocean floor)
    if (terrainHeight < seaLevel) {
        return false;
    }

    // Sample 3D noise with Y squashing for horizontal stretch
    float nx = static_cast<float>(worldX);
    float ny = static_cast<float>(worldY) * m_Config.ySquash;
    float nz = static_cast<float>(worldZ);

    float noiseValue = s_CaveNoise.GetNoise(nx, ny, nz);

    // Carve if noise is close to 0 (creates tube-like tunnels)
    return std::abs(noiseValue) < m_Config.threshold;
}

} // namespace Voxel
```

---

### Modified Files

#### [MODIFY] [TerrainGenerator.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/TerrainGenerator.h)

Add cave carver support:

```diff
 #pragma once

 #include "Chunk.h"
+#include "ICaveCarver.h"
 #include <cstdint>
+#include <vector>
+#include <memory>

 namespace Voxel {

 // Terrain generation configuration
 struct TerrainConfig {
     int seed = 12345;
     float frequency = 0.02f;
     int baseHeight = 64;
     int amplitude = 20;
     int seaLevel = 50;
+    bool enableCaves = true;  // Toggle cave generation
 };

 class TerrainGenerator {
 public:
     // ... existing constructors ...

+    // Add a cave carver (called during Generate)
+    void AddCaveCarver(std::unique_ptr<ICaveCarver> carver);
+    void ClearCaveCarvers();

 private:
     int GetHeightAt(int worldX, int worldZ) const;
+    void CarveCaves(Chunk& chunk, const std::vector<int>& heightMap);

     TerrainConfig m_Config;
+    std::vector<std::unique_ptr<ICaveCarver>> m_CaveCarvers;
 };
```

---

#### [MODIFY] [TerrainGenerator.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/TerrainGenerator.cpp)

Add two-pass generation with cave carving:

```diff
 void TerrainGenerator::Generate(Chunk& chunk) {
     int chunkOffsetX = chunk.GetChunkX() * CHUNK_WIDTH;
     int chunkOffsetZ = chunk.GetChunkZ() * CHUNK_DEPTH;

+    // Store height map for cave carving pass
+    std::vector<int> heightMap(CHUNK_WIDTH * CHUNK_DEPTH);

     // Pass 1: Generate terrain
     for (int x = 0; x < CHUNK_WIDTH; ++x) {
         for (int z = 0; z < CHUNK_DEPTH; ++z) {
             int worldX = chunkOffsetX + x;
             int worldZ = chunkOffsetZ + z;
             int height = GetHeightAt(worldX, worldZ);

+            heightMap[x + z * CHUNK_WIDTH] = height;

             // Fill column (existing logic)
             for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                 // ... existing block type logic ...
             }
         }
     }

+    // Pass 2: Carve caves
+    if (m_Config.enableCaves) {
+        CarveCaves(chunk, heightMap);
+    }

     chunk.SetDirty(true);
 }

+void TerrainGenerator::CarveCaves(Chunk& chunk, const std::vector<int>& heightMap) {
+    int chunkOffsetX = chunk.GetChunkX() * CHUNK_WIDTH;
+    int chunkOffsetZ = chunk.GetChunkZ() * CHUNK_DEPTH;
+
+    for (int x = 0; x < CHUNK_WIDTH; ++x) {
+        for (int z = 0; z < CHUNK_DEPTH; ++z) {
+            int worldX = chunkOffsetX + x;
+            int worldZ = chunkOffsetZ + z;
+            int terrainHeight = heightMap[x + z * CHUNK_WIDTH];
+
+            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
+                // Check all cave carvers
+                for (const auto& carver : m_CaveCarvers) {
+                    if (carver->ShouldCarve(worldX, y, worldZ,
+                                            terrainHeight, m_Config.seaLevel)) {
+                        // Carve: Air above sea level, Water below
+                        if (y < m_Config.seaLevel) {
+                            chunk.SetBlock(x, y, z, BlockType::Water);
+                        } else {
+                            chunk.SetBlock(x, y, z, BlockType::Air);
+                        }
+                        break; // One carver is enough
+                    }
+                }
+            }
+        }
+    }
+}

+void TerrainGenerator::AddCaveCarver(std::unique_ptr<ICaveCarver> carver) {
+    carver->Configure(m_Config.seed);
+    m_CaveCarvers.push_back(std::move(carver));
+}

+void TerrainGenerator::ClearCaveCarvers() {
+    m_CaveCarvers.clear();
+}
```

---

#### [MODIFY] [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt)

Add new cave files to build:

```diff
 set(WORLD_SOURCES
     world/Block.cpp
     world/Chunk.cpp
     world/ChunkMeshBuilder.cpp
     world/ChunkManager.cpp
     world/TerrainGenerator.cpp
+    world/SpaghettiCaveCarver.cpp
 )
```

---

#### [MODIFY] [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp)

Configure cave carver on startup:

```diff
+#include "SpaghettiCaveCarver.h"

 void Engine::SetupWorld() {
     TerrainConfig config;
     config.seed = 42;
     // ... existing config ...

+    // Add spaghetti cave carver
+    m_TerrainGenerator.AddCaveCarver(
+        std::make_unique<SpaghettiCaveCarver>()
+    );

     // ... rest of setup ...
 }
```

---

## Verification Plan

### Build Verification

- [ ] `cmake --build build` succeeds with no errors
- [ ] No new warnings in cave-related files

### Runtime Verification

- [ ] Caves appear underground (visible when flying below surface)
- [ ] Cave tunnels are winding and organic
- [ ] No caves break through terrain surface (surface protection works)
- [ ] Underwater caves are filled with water
- [ ] No visual regression for terrain generation
- [ ] Chunk boundaries don't create visible seams in caves

### Performance Check

- [ ] No noticeable performance degradation
- [ ] Chunk generation time is still reasonable

---

## Future Extensions (Not This Phase)

### Phase 8B: Cheese Caves

```cpp
class CheeseCaveCarver : public ICaveCarver { ... }
// Large open caverns with pillars
// Different noise frequency + higher threshold
```

### Phase 8C: Noodle Caves

```cpp
class NoodleCaveCarver : public ICaveCarver { ... }
// Very thin connecting tunnels
// High frequency, low threshold
```

### Phase 8D: Ore Generation

```cpp
class OreGenerator {
    void DistributeOres(Chunk& chunk);
    // Iron: Y=20-60, Coal: Y=10-80, Diamond: Y=5-16
}
```

---

## Task Checklist

- [ ] Create `world/ICaveCarver.h` (interface)
- [ ] Create `world/SpaghettiCaveCarver.h`
- [ ] Create `world/SpaghettiCaveCarver.cpp`
- [ ] Update `world/TerrainGenerator.h` (add carver support)
- [ ] Update `world/TerrainGenerator.cpp` (two-pass generation)
- [ ] Update `CMakeLists.txt` (add new sources)
- [ ] Update `core/Engine.cpp` (configure carver)
- [ ] Build and test
- [ ] Verify caves render correctly
- [ ] Update `.agent/task.md` with Phase 8A tasks
