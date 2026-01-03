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
    
    // Don't carve near surface (protect top layers from holes) - Minecraft uses ~4 blocks
    if (worldY >= terrainHeight - m_Config.surfaceProtection) {
        return false;
    }
    
    // Don't carve if this column is underwater (avoid carving ocean floor)
    if (terrainHeight < seaLevel) {
        return false;
    }
    
    // Altitude-based cave reduction (Minecraft-like)
    // Caves become rarer/thinner at higher elevations
    float altitudeThreshold = m_Config.threshold;
    if (worldY > Config::CAVE_FADE_START_Y) {
        // Linearly reduce threshold from FADE_START_Y to FADE_END_Y
        float fadeProgress = static_cast<float>(worldY - Config::CAVE_FADE_START_Y) 
                           / static_cast<float>(Config::CAVE_FADE_END_Y - Config::CAVE_FADE_START_Y);
        fadeProgress = std::min(fadeProgress, 1.0f);
        // Reduce threshold = narrower caves, eventually to 0 = no caves
        altitudeThreshold *= (1.0f - fadeProgress);
    }
    
    // Sample 3D Perlin noise with Y squashing for horizontal stretch
    float nx = static_cast<float>(worldX);
    float ny = static_cast<float>(worldY) * m_Config.ySquash;
    float nz = static_cast<float>(worldZ);
    
    float noiseValue = s_CaveNoise.GetNoise(nx, ny, nz);
    
    // Carve if noise value is close to zero (using altitude-adjusted threshold)
    return std::abs(noiseValue) < altitudeThreshold;
}

} // namespace Voxel
