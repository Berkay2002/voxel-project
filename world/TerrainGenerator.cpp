#include "TerrainGenerator.h"
#include "FastNoiseLite.h"

namespace Voxel {

// Static noise generator (configured on construction/SetConfig)
static FastNoiseLite s_Noise;

TerrainGenerator::TerrainGenerator() {
    SetConfig(TerrainConfig{});
}

TerrainGenerator::TerrainGenerator(const TerrainConfig& config) {
    SetConfig(config);
}

void TerrainGenerator::SetConfig(const TerrainConfig& config) {
    m_Config = config;
    
    // Configure FastNoiseLite
    s_Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_Noise.SetSeed(m_Config.seed);
    s_Noise.SetFrequency(m_Config.frequency);
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
    
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            // Calculate world position
            int worldX = chunkOffsetX + x;
            int worldZ = chunkOffsetZ + z;
            
            // Get terrain height at this column
            int height = GetHeightAt(worldX, worldZ);
            
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
    
    // Mark chunk as needing mesh rebuild
    chunk.SetDirty(true);
}

} // namespace Voxel
