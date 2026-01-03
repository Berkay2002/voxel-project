# Phase 3B: Terrain Generation

## Goal

Integrate FastNoiseLite and create a `TerrainGenerator` class to produce procedural height-based terrain with classic block layering.

---

## New Files

### world/TerrainGenerator.h/.cpp

- Owns a `FastNoiseLite` instance configured for Perlin noise
- `Generate(Chunk& chunk)` — fills chunk with procedural terrain
- Configurable seed, frequency, and amplitude

**Height Calculation:**

```cpp
float rawNoise = noise.GetNoise(worldX, worldZ);  // [-1, 1]
int height = BASE_HEIGHT + (int)(rawNoise * AMPLITUDE);
```

**Block Layering (Minecraft-style):**

- y = 0 → **Bedrock** (optional, or just Stone)
- y < height - 3 → **Stone**
- y < height → **Dirt**
- y == height → **Grass**
- y > height → **Air**

---

## Modified Files

### CMakeLists.txt

- Add FastNoiseLite via FetchContent:

```cmake
FetchContent_Declare(
  FastNoiseLite
  GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
  GIT_TAG v1.1.1
)
FetchContent_MakeAvailable(FastNoiseLite)
```

- Link/include FastNoiseLite header

### core/Engine.cpp

- Replace manual terrain with `TerrainGenerator::Generate(chunk)`
- Optionally expose noise seed/frequency for testing

---

## Configuration Defaults

| Parameter   | Value  | Notes                      |
| ----------- | ------ | -------------------------- |
| Noise Type  | Perlin | Simple, predictable        |
| Frequency   | 0.02   | Gentle hills               |
| BASE_HEIGHT | 64     | Sea-level equivalent       |
| AMPLITUDE   | 20     | Hills ±20 blocks           |
| Seed        | 12345  | Reproducible for debugging |

---

## Validation

```bash
cd build && cmake .. && make -j$(nproc) && ./VoxelEngine
```

- Terrain has rolling hills (not flat)
- Block layers visible: grass on top, dirt below, stone at depth
- Different areas of chunk have different heights
- Rerunning with same seed produces identical terrain

---

## Out of Scope (Deferred)

- Water / sea level
- Cave generation (3D noise)
- Multiple noise octaves
- Biomes
