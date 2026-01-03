#pragma once

#include <cstdint>

namespace Voxel {

/**
 * Abstract interface for cave carvers.
 * 
 * This modular design allows multiple cave types (spaghetti, cheese, noodle)
 * to be combined during terrain generation. Each carver independently decides
 * whether a block should be carved based on its own noise configuration.
 */
class ICaveCarver {
public:
    virtual ~ICaveCarver() = default;
    
    /**
     * Configure the carver with a world seed.
     * Called once when the carver is added to the terrain generator.
     * @param seed World seed (carvers should offset this to avoid correlation)
     */
    virtual void Configure(int seed) = 0;
    
    /**
     * Determine if a block at the given world position should be carved.
     * @param worldX World X coordinate
     * @param worldY World Y coordinate (height)
     * @param worldZ World Z coordinate
     * @param terrainHeight Surface height at this (X, Z) column
     * @param seaLevel World sea level (for underwater cave handling)
     * @return true if block should be carved (removed)
     */
    virtual bool ShouldCarve(int worldX, int worldY, int worldZ,
                             int terrainHeight, int seaLevel) const = 0;
    
    /**
     * Get display name for debugging/logging.
     */
    virtual const char* GetName() const = 0;
};

} // namespace Voxel
