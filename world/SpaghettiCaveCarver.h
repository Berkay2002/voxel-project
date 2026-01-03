#pragma once

#include "ICaveCarver.h"
#include "WorldConfig.h"

namespace Voxel {

/**
 * Configuration for spaghetti-style caves.
 * These are the classic Minecraft winding tunnel caves.
 * Default values come from WorldConfig.h for centralized tuning.
 */
struct SpaghettiCaveConfig {
    int minY = Config::CAVE_MIN_Y;
    int maxY = Config::CAVE_MAX_Y;
    int surfaceProtection = Config::CAVE_SURFACE_PROTECT;
    float frequency = Config::CAVE_FREQUENCY;
    float threshold = Config::CAVE_THRESHOLD;
    float ySquash = Config::CAVE_Y_SQUASH;
};

/**
 * Spaghetti cave carver using 3D Perlin noise.
 * 
 * Creates winding, tube-like tunnels by carving blocks where the noise
 * value is close to zero. The abs(noise) < threshold technique creates
 * connected passages that wind through the terrain.
 */
class SpaghettiCaveCarver : public ICaveCarver {
public:
    SpaghettiCaveCarver();
    explicit SpaghettiCaveCarver(const SpaghettiCaveConfig& config);
    
    // ICaveCarver interface
    void Configure(int seed) override;
    bool ShouldCarve(int worldX, int worldY, int worldZ,
                     int terrainHeight, int seaLevel) const override;
    const char* GetName() const override { return "SpaghettiCaves"; }
    
    // Configuration
    void SetConfig(const SpaghettiCaveConfig& config);
    const SpaghettiCaveConfig& GetConfig() const { return m_Config; }
    
private:
    SpaghettiCaveConfig m_Config;
};

} // namespace Voxel
