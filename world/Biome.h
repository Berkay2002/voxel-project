#pragma once

#include <cstdint>

namespace Voxel {

/**
 * Biome Types
 * 
 * Each biome defines distinct terrain characteristics.
 */
enum class BiomeType : uint8_t {
    Plains = 0,
    Mountains,
    COUNT
};

/**
 * Biome Parameters
 * 
 * Defines how terrain generates within each biome.
 */
struct BiomeParams {
    int baseHeight;      // Average terrain height
    int amplitude;       // Height variation (+/-)
    int grassDepth;      // Layers of grass/dirt before stone
    bool hasRivers;      // Whether rivers can spawn
};

/**
 * Get terrain parameters for a biome type.
 */
inline BiomeParams GetBiomeParams(BiomeType type) {
    switch (type) {
        case BiomeType::Plains:
            return {
                .baseHeight = 45,    // Low, flat terrain
                .amplitude = 8,      // Minimal variation
                .grassDepth = 4,     // Thick soil layer
                .hasRivers = true
            };
        case BiomeType::Mountains:
            return {
                .baseHeight = 65,    // High terrain
                .amplitude = 35,     // Dramatic peaks and valleys
                .grassDepth = 2,     // Thin soil, more exposed stone
                .hasRivers = false   // No rivers on mountains
            };
        default:
            return GetBiomeParams(BiomeType::Plains);
    }
}

/**
 * Get a human-readable name for a biome type.
 */
inline const char* GetBiomeName(BiomeType type) {
    switch (type) {
        case BiomeType::Plains:    return "Plains";
        case BiomeType::Mountains: return "Mountains";
        default:                   return "Unknown";
    }
}

} // namespace Voxel
