    #pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace Voxel {

// Block types available in the world
enum class BlockType : uint8_t {
    Air = 0,
    Dirt,
    Grass,
    Stone,
    Water,
    COUNT  // Keep last for iteration
};

// Face directions for mesh generation
enum class Face : uint8_t {
    Top = 0,     // +Y
    Bottom,      // -Y
    North,       // +Z
    South,       // -Z
    East,        // +X
    West         // -X
};

// Check if a block type is solid/opaque (blocks light and visibility)
// Water is NOT opaque - light passes through
inline bool IsOpaque(BlockType type) {
    return type != BlockType::Air && type != BlockType::Water;
}

// Check if a block should be rendered (has geometry)
inline bool IsSolid(BlockType type) {
    return type != BlockType::Air;
}

// Check if a block is transparent (rendered with alpha blending)
inline bool IsTransparent(BlockType type) {
    return type == BlockType::Water;
}

// Get the direction vector for a face
inline glm::ivec3 GetFaceDirection(Face face) {
    switch (face) {
        case Face::Top:    return { 0,  1,  0};
        case Face::Bottom: return { 0, -1,  0};
        case Face::North:  return { 0,  0,  1};
        case Face::South:  return { 0,  0, -1};
        case Face::East:   return { 1,  0,  0};
        case Face::West:   return {-1,  0,  0};
        default:           return { 0,  0,  0};
    }
}

// Get UV coordinates for a block face
// Returns bottom-left UV coordinate, each texture is 1x1 in UV space
// For now, simple mapping - can be expanded for texture atlas later
inline glm::vec2 GetBlockUV(BlockType type, Face face) {
    // Simple UV mapping - all faces use full texture
    // In future, this will return atlas coordinates
    switch (type) {
        case BlockType::Grass:
            // Grass has different textures per face
            if (face == Face::Top) {
                return {0.0f, 0.0f};  // Grass top texture
            } else if (face == Face::Bottom) {
                return {0.0f, 0.0f};  // Dirt texture for bottom
            } else {
                return {0.0f, 0.0f};  // Grass side texture
            }
        case BlockType::Dirt:
        case BlockType::Stone:
        default:
            return {0.0f, 0.0f};  // All faces same texture
    }
}

// Get texture index for a block face (for texture array/atlas)
inline int GetTextureIndex(BlockType type, Face face) {
    switch (type) {
        case BlockType::Grass:
            if (face == Face::Top) return 0;        // grass_block.png
            if (face == Face::Bottom) return 1;     // dirt_block.png
            return 2;                               // grass_block_side.png
        case BlockType::Dirt:
            return 1;  // dirt_block.png
        case BlockType::Stone:
            return 3;  // stone_block.png
        case BlockType::Water:
            return 4;  // water.png
        default:
            return 0;
    }
}

// Get color for a block face (used for per-vertex coloring before texture atlas)
inline glm::vec3 GetBlockColor(BlockType type, Face face) {
    switch (type) {
        case BlockType::Grass:
            if (face == Face::Top) {
                return glm::vec3(0.45f, 0.75f, 0.35f);  // Green grass top
            } else if (face == Face::Bottom) {
                return glm::vec3(0.55f, 0.35f, 0.20f);  // Dirt bottom
            } else {
                // Grass sides: gradient from grass to dirt
                return glm::vec3(0.50f, 0.55f, 0.30f);  // Brownish-green
            }
        case BlockType::Dirt:
            return glm::vec3(0.55f, 0.35f, 0.20f);      // Brown dirt
        case BlockType::Stone:
            return glm::vec3(0.55f, 0.55f, 0.55f);      // Gray stone
        case BlockType::Water:
            return glm::vec3(0.2f, 0.4f, 0.8f);         // Blue-ish water
        default:
            return glm::vec3(1.0f, 0.0f, 1.0f);         // Magenta for missing
    }
}

} // namespace Voxel
