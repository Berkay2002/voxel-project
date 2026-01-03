#include "TerrainGenerator.h"
#include "FastNoiseLite.h"

namespace Voxel {

// Static noise generator for terrain (configured on construction/SetConfig)
static FastNoiseLite s_Noise;

TerrainGenerator::TerrainGenerator() {
    SetConfig(TerrainConfig{});
}

TerrainGenerator::TerrainGenerator(const TerrainConfig& config) {
    SetConfig(config);
}

void TerrainGenerator::SetConfig(const TerrainConfig& config) {
    m_Config = config;
    
    // Configure FastNoiseLite for terrain
    s_Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_Noise.SetSeed(m_Config.seed);
    s_Noise.SetFrequency(m_Config.frequency);
    
    // Reconfigure all cave carvers with new seed
    for (auto& carver : m_CaveCarvers) {
        carver->Configure(m_Config.seed);
    }
}

int TerrainGenerator::GetHeightAt(int worldX, int worldZ) const {
    // Get noise value in range [-1, 1]
    float noiseValue = s_Noise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    // Convert to height: baseHeight + (noise * amplitude)
    int height = m_Config.baseHeight + static_cast<int>(noiseValue * m_Config.amplitude);
    
    // Clamp to valid chunk height range
    if (height < 1) height = 1;
    if (height >= CHUNK_HEIGHT - 1) height = CHUNK_HEIGHT - 2;
    
    return height;
}

void TerrainGenerator::Generate(Chunk& chunk) {
    // Get chunk world offset (for multi-chunk support)
    int chunkOffsetX = chunk.GetChunkX() * CHUNK_WIDTH;
    int chunkOffsetZ = chunk.GetChunkZ() * CHUNK_DEPTH;
    
    // Store height map for cave carving pass
    std::vector<int> heightMap(CHUNK_WIDTH * CHUNK_DEPTH);
    
    // Pass 1: Generate base terrain
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            // Calculate world position
            int worldX = chunkOffsetX + x;
            int worldZ = chunkOffsetZ + z;
            
            // Get terrain height at this column
            int height = GetHeightAt(worldX, worldZ);
            heightMap[x + z * CHUNK_WIDTH] = height;
            
            // Fill column with blocks using Minecraft-style layering
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                BlockType type = BlockType::Air;
                
                if (y < height - 3) {
                    // Deep underground: Stone
                    type = BlockType::Stone;
                } else if (y < height) {
                    // Near surface: Dirt
                    type = BlockType::Dirt;
                } else if (y == height) {
                    // Surface: Grass (or Sand if below water, future enhancement)
                    type = BlockType::Grass;
                } else if (y <= m_Config.seaLevel && y > height) {
                    // Above terrain but at or below sea level: Water
                    type = BlockType::Water;
                }
                // y > seaLevel && y > height: Air (already initialized)
                
                chunk.SetBlock(x, y, z, type);
            }
        }
    }
    
    // Pass 2: Carve caves (if enabled and carvers exist)
    if (m_Config.enableCaves && !m_CaveCarvers.empty()) {
        CarveCaves(chunk, heightMap);
    }
    
    // Mark chunk as needing mesh rebuild
    chunk.SetDirty(true);
}

void TerrainGenerator::CarveCaves(Chunk& chunk, const std::vector<int>& heightMap) {
    int chunkOffsetX = chunk.GetChunkX() * CHUNK_WIDTH;
    int chunkOffsetZ = chunk.GetChunkZ() * CHUNK_DEPTH;
    
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = chunkOffsetX + x;
            int worldZ = chunkOffsetZ + z;
            int terrainHeight = heightMap[x + z * CHUNK_WIDTH];
            
            // Only iterate through underground blocks for efficiency
            for (int y = 0; y < terrainHeight; ++y) {
                // Check each cave carver
                for (const auto& carver : m_CaveCarvers) {
                    if (carver->ShouldCarve(worldX, y, worldZ, 
                                            terrainHeight, m_Config.seaLevel)) {
                        // Carve the block:
                        // - Below sea level: fill with Water (flooded caves)
                        // - Above sea level: set to Air (open caves)
                        if (y < m_Config.seaLevel) {
                            chunk.SetBlock(x, y, z, BlockType::Water);
                        } else {
                            chunk.SetBlock(x, y, z, BlockType::Air);
                        }
                        break; // One carver is enough to carve this block
                    }
                }
            }
        }
    }
}

void TerrainGenerator::AddCaveCarver(std::unique_ptr<ICaveCarver> carver) {
    // Configure the carver with current seed before adding
    carver->Configure(m_Config.seed);
    m_CaveCarvers.push_back(std::move(carver));
}

void TerrainGenerator::ClearCaveCarvers() {
    m_CaveCarvers.clear();
}

} // namespace Voxel
