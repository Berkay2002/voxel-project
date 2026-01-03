#pragma once

/**
 * Centralized World Configuration
 * 
 * All tunable parameters for terrain generation, caves, and world features.
 * Edit this file to adjust world generation without hunting through multiple files.
 */

namespace Voxel {
namespace Config {

// =============================================================================
// TERRAIN GENERATION
// =============================================================================

// General terrain shape
constexpr int TERRAIN_SEED         = 12345;
constexpr float TERRAIN_FREQUENCY  = 0.02f;   // Lower = larger features
constexpr int TERRAIN_BASE_HEIGHT  = 55;      // Average terrain height
constexpr int TERRAIN_AMPLITUDE    = 25;      // Height variation (+/-), range: ~30-80

// Water
constexpr int SEA_LEVEL            = 35;      // Water fills below this (low = less water)

// =============================================================================
// CAVE GENERATION - SPAGHETTI CAVES
// =============================================================================

// Minecraft-style winding tunnel caves
constexpr int CAVE_MIN_Y           = 5;       // Lowest cave level (above "bedrock")
constexpr int CAVE_MAX_Y           = 90;      // Must exceed max terrain (~80) for hillside entrances
constexpr int CAVE_SURFACE_PROTECT = 4;       // Protect top N blocks from cave holes
constexpr int CAVE_FADE_START_Y    = 50;      // Caves start fading above this Y
constexpr int CAVE_FADE_END_Y      = 80;      // No caves above this Y

// Noise parameters
constexpr float CAVE_FREQUENCY     = 0.03f;   // Lower = larger, more connected features
constexpr float CAVE_THRESHOLD     = 0.22f;   // Higher = wider tunnels (0.15-0.25 typical)
constexpr float CAVE_Y_SQUASH      = 2.0f;    // > 1 = horizontally stretched (wider tunnels)

// =============================================================================
// CHUNK LOADING
// =============================================================================

constexpr int CHUNK_LOAD_RADIUS    = 8;       // Chunks loaded around player
constexpr int CHUNK_UNLOAD_RADIUS  = 10;      // Chunks unloaded beyond this
constexpr int MAX_MESH_UPLOADS_PER_FRAME = 2; // Rate limit GPU uploads

// =============================================================================
// BIOME GENERATION
// =============================================================================

constexpr float BIOME_FREQUENCY    = 0.005f;  // Large-scale biome regions (lower = bigger biomes)

// Plains biome parameters
constexpr int PLAINS_BASE_HEIGHT   = 45;      // Low, flat terrain
constexpr int PLAINS_AMPLITUDE     = 8;       // Minimal variation

// Mountains biome parameters  
constexpr int MOUNTAINS_BASE_HEIGHT = 65;     // High terrain
constexpr int MOUNTAINS_AMPLITUDE   = 35;     // Dramatic peaks

// =============================================================================
// RIVER GENERATION
// =============================================================================

constexpr float RIVER_FREQUENCY    = 0.015f;  // River winding scale
constexpr float RIVER_THRESHOLD    = 0.88f;   // Higher = narrower rivers (0.85-0.95)
constexpr int RIVER_DEPTH          = 4;       // How deep rivers carve into terrain
constexpr int RIVER_WATER_LEVEL    = 2;       // Water depth in rivers

// =============================================================================
// FEATURES TOGGLE
// =============================================================================

constexpr bool ENABLE_CAVES        = true;
constexpr bool ENABLE_RIVERS       = true;

} // namespace Config
} // namespace Voxel
