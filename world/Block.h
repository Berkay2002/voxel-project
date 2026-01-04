#pragma once

/**
 * Block.h - Backward Compatibility Layer
 * 
 * This file provides backward compatibility with the old BlockType enum
 * while the codebase transitions to the new BlockRegistry system.
 * 
 * New code should use:
 *   - BlockRegistry::Instance().GetBlockID("grass_block")
 *   - BlockRegistry::Instance().GetTextureIndex(blockId, face)
 * 
 * Legacy code using BlockType enum will continue to work.
 */

#include "BlockRegistry.h"

namespace Voxel {

// Legacy BlockType enum for backward compatibility
// Maps directly to BlockID values from blocks.json load order
enum class BlockType : uint16_t {
    Air = 0,
    Grass = 1,      // "grass_block" in JSON
    Dirt = 2,       // "dirt" in JSON
    Stone = 3,      // "stone" in JSON
    Water = 4,      // "water" in JSON
    Bedrock = 5,    // "bedrock" in JSON
    Sand = 6,       // "sand" in JSON
    Gravel = 7,     // "gravel" in JSON
    Cobblestone = 8,// "cobblestone" in JSON
    OakLog = 9,     // "oak_log" in JSON
    OakPlanks = 10, // "oak_planks" in JSON
    OakLeaves = 11, // "oak_leaves" in JSON
    CoalOre = 12,   // "coal_ore" in JSON
    IronOre = 13,   // "iron_ore" in JSON
    GoldOre = 14,   // "gold_ore" in JSON
    DiamondOre = 15,// "diamond_ore" in JSON
    CopperOre = 16, // "copper_ore" in JSON
    EmeraldOre = 17,// "emerald_ore" in JSON
    COUNT           // Keep last for iteration
};

// Convert BlockType to BlockID
inline BlockID ToBlockID(BlockType type) {
    return static_cast<BlockID>(type);
}

// Convert BlockID to BlockType (for legacy code)
inline BlockType ToBlockType(BlockID id) {
    if (id >= static_cast<BlockID>(BlockType::COUNT)) {
        return BlockType::Air;
    }
    return static_cast<BlockType>(id);
}

// Legacy helper functions that delegate to BlockRegistry
// These allow existing code to continue working without changes

inline bool IsOpaque(BlockType type) {
    return BlockRegistry::Instance().IsOpaque(ToBlockID(type));
}

inline bool IsSolid(BlockType type) {
    return BlockRegistry::Instance().IsSolid(ToBlockID(type));
}

inline bool IsTransparent(BlockType type) {
    return BlockRegistry::Instance().IsTransparent(ToBlockID(type));
}

inline int GetTextureIndex(BlockType type, Face face) {
    return BlockRegistry::Instance().GetTextureIndex(ToBlockID(type), face);
}

// Legacy UV function (kept for compatibility, returns 0,0 as before)
inline glm::vec2 GetBlockUV([[maybe_unused]] BlockType type, [[maybe_unused]] Face face) {
    return {0.0f, 0.0f};
}

// Legacy color function (kept for debug/fallback, not used with textures)
inline glm::vec3 GetBlockColor(BlockType type, [[maybe_unused]] Face face) {
    switch (type) {
        case BlockType::Grass:
            return glm::vec3(0.45f, 0.75f, 0.35f);
        case BlockType::Dirt:
            return glm::vec3(0.55f, 0.35f, 0.20f);
        case BlockType::Stone:
            return glm::vec3(0.55f, 0.55f, 0.55f);
        case BlockType::Water:
            return glm::vec3(0.2f, 0.4f, 0.8f);
        case BlockType::Sand:
            return glm::vec3(0.85f, 0.80f, 0.55f);
        case BlockType::Gravel:
            return glm::vec3(0.5f, 0.5f, 0.5f);
        case BlockType::Cobblestone:
            return glm::vec3(0.45f, 0.45f, 0.45f);
        default:
            return glm::vec3(1.0f, 0.0f, 1.0f); // Magenta for missing
    }
}

} // namespace Voxel
