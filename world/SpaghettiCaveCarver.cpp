#include "SpaghettiCaveCarver.h"
#include "FastNoiseLite.h"
#include <cmath>

namespace Voxel {

// Static noise generator for cave carving
// Using static to avoid per-instance overhead
static FastNoiseLite s_CaveNoise;
static bool s_NoiseConfigured = false;

SpaghettiCaveCarver::SpaghettiCaveCarver() {
    SetConfig(SpaghettiCaveConfig{});
}

SpaghettiCaveCarver::SpaghettiCaveCarver(const SpaghettiCaveConfig& config) {
    SetConfig(config);
}

void SpaghettiCaveCarver::Configure(int seed) {
    s_CaveNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_CaveNoise.SetSeed(seed + 1000);  // Offset from terrain seed to decorrelate
    s_CaveNoise.SetFrequency(m_Config.frequency);
    s_NoiseConfigured = true;
}

void SpaghettiCaveCarver::SetConfig(const SpaghettiCaveConfig& config) {
    m_Config = config;
    
    // Update noise frequency if already configured
    if (s_NoiseConfigured) {
        s_CaveNoise.SetFrequency(m_Config.frequency);
    }
}

bool SpaghettiCaveCarver::ShouldCarve(int worldX, int worldY, int worldZ,
                                       int terrainHeight, int seaLevel) const {
    // Don't carve outside configured Y range
    if (worldY < m_Config.minY || worldY > m_Config.maxY) {
        return false;
    }
    
    // Don't carve near surface (protect top layers from holes)
    if (worldY >= terrainHeight - m_Config.surfaceProtection) {
        return false;
    }
    
    // Don't carve if this column is underwater (avoid carving ocean floor)
    // This prevents weird underwater terrain artifacts
    if (terrainHeight < seaLevel) {
        return false;
    }
    
    // Sample 3D Perlin noise with Y squashing for horizontal stretch
    // Multiplying Y by ySquash (1.5) makes caves wider than tall
    float nx = static_cast<float>(worldX);
    float ny = static_cast<float>(worldY) * m_Config.ySquash;
    float nz = static_cast<float>(worldZ);
    
    float noiseValue = s_CaveNoise.GetNoise(nx, ny, nz);
    
    // Carve if noise value is close to zero
    // This creates tube-like tunnels along the "zero isosurface"
    // Lower threshold = thinner tunnels, higher = wider caves
    return std::abs(noiseValue) < m_Config.threshold;
}

} // namespace Voxel
