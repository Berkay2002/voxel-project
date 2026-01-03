#pragma once

#include "Chunk.h"
#include "ICaveCarver.h"
#include "WorldConfig.h"
#include <cstdint>
#include <vector>
#include <memory>

namespace Voxel {

// Terrain generation configuration
// Default values come from WorldConfig.h for centralized tuning.
struct TerrainConfig {
    int seed = Config::TERRAIN_SEED;
    float frequency = Config::TERRAIN_FREQUENCY;
    int baseHeight = Config::TERRAIN_BASE_HEIGHT;
    int amplitude = Config::TERRAIN_AMPLITUDE;
    int seaLevel = Config::SEA_LEVEL;
    bool enableCaves = Config::ENABLE_CAVES;
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

    // Cave carver management
    void AddCaveCarver(std::unique_ptr<ICaveCarver> carver);
    void ClearCaveCarvers();
    size_t GetCaveCarverCount() const { return m_CaveCarvers.size(); }

private:
    // Calculate terrain height at world position (x, z)
    int GetHeightAt(int worldX, int worldZ) const;

    // Carve caves into the chunk using registered carvers
    void CarveCaves(Chunk& chunk, const std::vector<int>& heightMap);

    TerrainConfig m_Config;
    std::vector<std::unique_ptr<ICaveCarver>> m_CaveCarvers;
};

} // namespace Voxel
