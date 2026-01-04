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
// ORE GENERATION
// =============================================================================

// Ore noise settings
constexpr float ORE_BASE_FREQUENCY = 0.1f;   // Base noise frequency for ore blobs

// Ore distribution: {maxY, threshold, veinFreq}
// - maxY: Maximum spawn height
// - threshold: Rarity (0.88 = common, 0.98 = very rare)
// - veinFreq: Vein size multiplier (0.08 = large veins, 0.20 = tiny veins)

// Coal: very common, large veins, spawns high
constexpr int   COAL_MAX_Y       = 80;
constexpr float COAL_THRESHOLD   = 0.88f;
constexpr float COAL_VEIN_FREQ   = 0.08f;

// Iron: fairly common, medium veins
constexpr int   IRON_MAX_Y       = 64;
constexpr float IRON_THRESHOLD   = 0.91f;
constexpr float IRON_VEIN_FREQ   = 0.10f;

// Copper: moderate rarity
constexpr int   COPPER_MAX_Y     = 48;
constexpr float COPPER_THRESHOLD = 0.92f;
constexpr float COPPER_VEIN_FREQ = 0.11f;

// Gold: uncommon, smaller veins, deep
constexpr int   GOLD_MAX_Y       = 32;
constexpr float GOLD_THRESHOLD   = 0.95f;
constexpr float GOLD_VEIN_FREQ   = 0.14f;

// Emerald: rare, tiny veins (almost single blocks)
constexpr int   EMERALD_MAX_Y     = 32;
constexpr float EMERALD_THRESHOLD = 0.975f;
constexpr float EMERALD_VEIN_FREQ = 0.20f;

// Diamond: very rare, small veins, very deep only
constexpr int   DIAMOND_MAX_Y     = 16;
constexpr float DIAMOND_THRESHOLD = 0.965f;
constexpr float DIAMOND_VEIN_FREQ = 0.16f;

// =============================================================================
// DISTANCE FOG
// =============================================================================

constexpr float FOG_START           = 80.0f;   // Start fading at this distance (blocks)
constexpr float FOG_END             = 128.0f;  // Fully fogged at this distance (blocks)
// Fog color matches sky: RGB (0.5, 0.7, 1.0) - set in Engine.cpp

// =============================================================================
// FEATURES TOGGLE
// =============================================================================

constexpr bool ENABLE_CAVES        = true;
constexpr bool ENABLE_RIVERS       = true;
constexpr bool ENABLE_ORES         = true;

} // namespace Config
} // namespace Voxel
