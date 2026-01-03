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
constexpr int CAVE_SURFACE_PROTECT = 0;       // 0 = caves can carve to surface (natural entrances)

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
// FEATURES TOGGLE
// =============================================================================

constexpr bool ENABLE_CAVES        = true;

} // namespace Config
} // namespace Voxel
