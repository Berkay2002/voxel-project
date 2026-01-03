#pragma once

#include "Chunk.h"
#include <cstdint>

namespace Voxel {

// Terrain generation configuration
struct TerrainConfig {
    int seed = 12345;
    float frequency = 0.02f;    // Lower = larger features
    int baseHeight = 64;        // Average terrain height
    int amplitude = 20;         // Height variation (+/-)
    int seaLevel = 50;          // Water fills below this level (valleys get water)
};

class TerrainGenerator {
public:
    TerrainGenerator();
    explicit TerrainGenerator(const TerrainConfig& config);
    ~TerrainGenerator() = default;

    // Generate terrain for a chunk at the given chunk coordinates
    void Generate(Chunk& chunk);

    // Configuration
    void SetConfig(const TerrainConfig& config);
    const TerrainConfig& GetConfig() const { return m_Config; }

private:
    // Calculate terrain height at world position (x, z)
    int GetHeightAt(int worldX, int worldZ) const;

    TerrainConfig m_Config;
};

} // namespace Voxel
