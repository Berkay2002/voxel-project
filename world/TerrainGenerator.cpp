#include "TerrainGenerator.h"
#include "FastNoiseLite.h"
#include <algorithm>  // for std::clamp

namespace Voxel {

// Static noise generators
static FastNoiseLite s_TerrainNoise;  // Height map
static FastNoiseLite s_BiomeNoise;    // Biome selection
static FastNoiseLite s_RiverNoise;    // River paths
static FastNoiseLite s_OreNoise;      // Ore distribution

TerrainGenerator::TerrainGenerator() {
    SetConfig(TerrainConfig{});
}

TerrainGenerator::TerrainGenerator(const TerrainConfig& config) {
    SetConfig(config);
}

void TerrainGenerator::SetConfig(const TerrainConfig& config) {
    m_Config = config;
    
    // Configure terrain noise
    s_TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_TerrainNoise.SetSeed(m_Config.seed);
    s_TerrainNoise.SetFrequency(m_Config.frequency);
    
    // Configure biome noise (larger scale for biome regions)
    s_BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_BiomeNoise.SetSeed(m_Config.seed + 1000);  // Different seed offset
    s_BiomeNoise.SetFrequency(Config::BIOME_FREQUENCY);
    
    // Configure river noise (using cellular for winding paths)
    s_RiverNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    s_RiverNoise.SetSeed(m_Config.seed + 2000);
    s_RiverNoise.SetFrequency(Config::RIVER_FREQUENCY);
    s_RiverNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
    
    // Configure ore noise (3D noise for blob-like distribution)
    s_OreNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    s_OreNoise.SetSeed(m_Config.seed + 3000);
    s_OreNoise.SetFrequency(0.1f);  // Controls ore blob size
    
    // Reconfigure all cave carvers with new seed
    for (auto& carver : m_CaveCarvers) {
        carver->Configure(m_Config.seed);
    }
}

BiomeType TerrainGenerator::GetBiomeAt(int worldX, int worldZ) const {
    // Sample biome noise: range [-1, 1]
    float noiseValue = s_BiomeNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    // Convert to biome: positive = Mountains, negative = Plains
    // Note: For river checks we still need discrete biome type
    if (noiseValue > 0.1f) {
        return BiomeType::Mountains;
    }
    return BiomeType::Plains;
}

// Helper: smoothstep for smooth interpolation
static float Smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Get blended biome parameters at world position (smooth transitions)
static BiomeParams GetBlendedBiomeParams(int worldX, int worldZ) {
    // Sample biome noise
    float noiseValue = s_BiomeNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    BiomeParams plains = GetBiomeParams(BiomeType::Plains);
    BiomeParams mountains = GetBiomeParams(BiomeType::Mountains);
    
    // Transition zone: noise [-0.2, 0.3] blends between biomes
    // Below -0.2 = pure Plains, above 0.3 = pure Mountains
    float blend = Smoothstep(-0.2f, 0.3f, noiseValue);
    
    // Interpolate parameters
    BiomeParams blended;
    blended.baseHeight = static_cast<int>(plains.baseHeight * (1.0f - blend) + mountains.baseHeight * blend);
    blended.amplitude = static_cast<int>(plains.amplitude * (1.0f - blend) + mountains.amplitude * blend);
    blended.grassDepth = static_cast<int>(plains.grassDepth * (1.0f - blend) + mountains.grassDepth * blend);
    blended.hasRivers = blend < 0.5f;  // Rivers only in mostly-Plains areas
    
    return blended;
}

bool TerrainGenerator::IsRiver(int worldX, int worldZ) const {
    if (!m_Config.enableRivers) return false;
    
    // Cellular noise creates natural-looking winding paths
    float noiseValue = s_RiverNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    // Normalize to [0, 1] and check threshold
    // Values close to 0 mark the cell edges = river paths
    float normalized = (noiseValue + 1.0f) * 0.5f;
    return normalized < (1.0f - Config::RIVER_THRESHOLD);
}

int TerrainGenerator::GetRiverDepth(int worldX, int worldZ) const {
    if (!IsRiver(worldX, worldZ)) return 0;
    
    // Rivers carve down from terrain
    return Config::RIVER_DEPTH;
}

int TerrainGenerator::GetHeightAt(int worldX, int worldZ) const {
    // Get BLENDED biome parameters (smooth transitions!)
    BiomeParams params = GetBlendedBiomeParams(worldX, worldZ);
    
    // Get noise value in range [-1, 1]
    float noiseValue = s_TerrainNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    // Convert to height using blended parameters
    int height = params.baseHeight + static_cast<int>(noiseValue * params.amplitude);
    
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
    
    // Pass 1: Generate base terrain with biomes
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            // Calculate world position
            int worldX = chunkOffsetX + x;
            int worldZ = chunkOffsetZ + z;
            
            // Get BLENDED biome params and terrain height
            BiomeParams biomeParams = GetBlendedBiomeParams(worldX, worldZ);
            int height = GetHeightAt(worldX, worldZ);
            
            // Check for river at this position
            int riverDepth = GetRiverDepth(worldX, worldZ);
            bool isRiver = riverDepth > 0 && biomeParams.hasRivers;
            int riverBed = height - riverDepth;
            
            heightMap[x + z * CHUNK_WIDTH] = isRiver ? riverBed : height;
            
            // Fill column with blocks using biome-specific layering
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                BlockType type = BlockType::Air;
                
                if (isRiver && y > riverBed && y <= riverBed + Config::RIVER_WATER_LEVEL) {
                    // River water
                    type = BlockType::Water;
                } else if (y < height - biomeParams.grassDepth) {
                    // Deep underground: Stone
                    type = BlockType::Stone;
                } else if (y < height) {
                    // Near surface: Dirt
                    type = BlockType::Dirt;
                } else if (y == height && (!isRiver || y > riverBed + Config::RIVER_WATER_LEVEL)) {
                    // Surface: Grass (but not underwater)
                    type = BlockType::Grass;
                }
                // Above terrain: Air (already initialized)
                
                chunk.SetBlock(x, y, z, type);
            }
        }
    }
    
    // Pass 2: Carve caves (if enabled and carvers exist)
    if (m_Config.enableCaves && !m_CaveCarvers.empty()) {
        CarveCaves(chunk, heightMap);
    }
    
    // Pass 3: Generate ores in stone
    GenerateOres(chunk, heightMap);
    
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
                        // Carve the block to Air (caves are air-filled)
                        chunk.SetBlock(x, y, z, BlockType::Air);
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

void TerrainGenerator::GenerateOres(Chunk& chunk, const std::vector<int>& heightMap) {
    int chunkOffsetX = chunk.GetChunkX() * CHUNK_WIDTH;
    int chunkOffsetZ = chunk.GetChunkZ() * CHUNK_DEPTH;
    
    // Ore generation parameters from WorldConfig.h
    struct OreConfig {
        BlockType type;
        int maxY;
        float threshold;
        float veinFreq;
    };
    
    // Use centralized config values
    std::vector<OreConfig> ores = {
        {BlockType::CoalOre,     Config::COAL_MAX_Y,     Config::COAL_THRESHOLD,     Config::COAL_VEIN_FREQ},
        {BlockType::IronOre,     Config::IRON_MAX_Y,     Config::IRON_THRESHOLD,     Config::IRON_VEIN_FREQ},
        {BlockType::CopperOre,   Config::COPPER_MAX_Y,   Config::COPPER_THRESHOLD,   Config::COPPER_VEIN_FREQ},
        {BlockType::GoldOre,     Config::GOLD_MAX_Y,     Config::GOLD_THRESHOLD,     Config::GOLD_VEIN_FREQ},
        {BlockType::EmeraldOre,  Config::EMERALD_MAX_Y,  Config::EMERALD_THRESHOLD,  Config::EMERALD_VEIN_FREQ},
        {BlockType::DiamondOre,  Config::DIAMOND_MAX_Y,  Config::DIAMOND_THRESHOLD,  Config::DIAMOND_VEIN_FREQ},
    };
    
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = chunkOffsetX + x;
            int worldZ = chunkOffsetZ + z;
            int terrainHeight = heightMap[x + z * CHUNK_WIDTH];
            
            // Only generate ores in underground stone (not near surface)
            for (int y = 1; y < terrainHeight - 5; ++y) {
                // Only replace stone blocks
                if (chunk.GetBlock(x, y, z) != BlockType::Stone) {
                    continue;
                }
                
                // Check each ore type
                for (const auto& ore : ores) {
                    if (y > ore.maxY) continue;  // Skip if above max height
                    
                    // Sample noise with ore-specific frequency for vein size
                    float veinNoise = s_OreNoise.GetNoise(
                        static_cast<float>(worldX) * ore.veinFreq / 0.1f,
                        static_cast<float>(y) * ore.veinFreq / 0.1f,
                        static_cast<float>(worldZ) * ore.veinFreq / 0.1f
                    );
                    
                    // Add ore-type-specific offset for variation
                    float oreOffset = s_OreNoise.GetNoise(
                        static_cast<float>(worldX + static_cast<int>(ore.type) * 1000),
                        static_cast<float>(y),
                        static_cast<float>(worldZ + static_cast<int>(ore.type) * 500)
                    );
                    
                    // Combine noises and normalize
                    float combined = (veinNoise + oreOffset * 0.5f) / 1.5f;
                    float normalized = (combined + 1.0f) * 0.5f;
                    
                    if (normalized > ore.threshold) {
                        chunk.SetBlock(x, y, z, ore.type);
                        break;  // Only one ore type per block
                    }
                }
            }
        }
    }
}

} // namespace Voxel


