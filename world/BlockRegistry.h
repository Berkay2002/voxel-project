#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

namespace Core {
    class TextureManager;
    using TextureRegistry = TextureManager;  // Backwards compatibility
}

namespace Voxel {

// Block ID type - uint16_t allows 65535 block types
using BlockID = uint16_t;

// Special block IDs
constexpr BlockID BLOCK_AIR = 0;
constexpr BlockID BLOCK_INVALID = 65535;

// Face directions for mesh generation
enum class Face : uint8_t {
    Top = 0,     // +Y
    Bottom,      // -Y
    North,       // +Z
    South,       // -Z
    East,        // +X
    West         // -X
};

/**
 * Block definition loaded from JSON configuration.
 * Contains all properties and texture mappings for a block type.
 */
struct BlockDef {
    BlockID id = BLOCK_INVALID;
    std::string stringId;           // "grass_block", "oak_log", etc.
    
    // Properties
    bool solid = true;              // Has geometry (renderable)
    bool opaque = true;             // Blocks light and visibility
    bool transparent = false;       // Rendered with alpha blending
    bool tinted = false;            // Texture is grayscale, needs biome tint color
    
    // Default tint color (multiplied with texture)
    // White = no tint, used for blocks that aren't biome-tinted
    glm::vec3 tintColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Per-face tint flags (grass_block has tinted top but not bottom)
    bool tintTop = false;
    bool tintSides = false;
    
    // Texture layer indices (resolved from TextureRegistry)
    int textureTop = 0;
    int textureBottom = 0;
    int textureNorth = 0;
    int textureSouth = 0;
    int textureEast = 0;
    int textureWest = 0;
    
    // Get texture index for a specific face
    [[nodiscard]] int GetTextureIndex(Face face) const {
        switch (face) {
            case Face::Top:    return textureTop;
            case Face::Bottom: return textureBottom;
            case Face::North:  return textureNorth;
            case Face::South:  return textureSouth;
            case Face::East:   return textureEast;
            case Face::West:   return textureWest;
            default:           return textureTop;
        }
    }
    
    // Check if a specific face should be tinted
    [[nodiscard]] bool IsFaceTinted(Face face) const {
        if (!tinted) return false;
        if (face == Face::Top) return tintTop;
        if (face == Face::Bottom) return false;  // Bottom never tinted
        return tintSides;  // North, South, East, West
    }
    
    // Get tint color for a face
    [[nodiscard]] glm::vec3 GetFaceTintColor(Face face) const {
        return IsFaceTinted(face) ? tintColor : glm::vec3(1.0f);
    }
};

/**
 * BlockRegistry is a singleton that manages all block definitions.
 * It loads block configurations from JSON and provides fast lookups.
 * 
 * Usage:
 *   BlockRegistry& registry = BlockRegistry::Instance();
 *   registry.LoadFromFile("assets/config/blocks.json", textureRegistry);
 *   BlockID grassId = registry.GetBlockID("grass_block");
 *   const BlockDef& def = registry.GetBlockDef(grassId);
 */
class BlockRegistry {
public:
    // Singleton access
    static BlockRegistry& Instance();

    // Delete copy/move for singleton
    BlockRegistry(const BlockRegistry&) = delete;
    BlockRegistry& operator=(const BlockRegistry&) = delete;
    BlockRegistry(BlockRegistry&&) = delete;
    BlockRegistry& operator=(BlockRegistry&&) = delete;

    /**
     * Load block definitions from a JSON file.
     * @param path Path to blocks.json
     * @param textureRegistry Reference to TextureRegistry for resolving texture names
     * @return true if loaded successfully
     */
    bool LoadFromFile(const std::string& path, Core::TextureRegistry& textureRegistry);

    /**
     * Get block ID from string identifier.
     * @param stringId Block string ID (e.g., "grass_block")
     * @return Block ID, or BLOCK_INVALID if not found
     */
    [[nodiscard]] BlockID GetBlockID(const std::string& stringId) const;

    /**
     * Get block definition by ID.
     * @param id Block ID
     * @return Reference to block definition (returns air if invalid)
     */
    [[nodiscard]] const BlockDef& GetBlockDef(BlockID id) const;

    /**
     * Get texture index for a block face.
     * @param id Block ID
     * @param face Face direction
     * @return Texture layer index
     */
    [[nodiscard]] int GetTextureIndex(BlockID id, Face face) const;

    // Property queries
    [[nodiscard]] bool IsOpaque(BlockID id) const;
    [[nodiscard]] bool IsSolid(BlockID id) const;
    [[nodiscard]] bool IsTransparent(BlockID id) const;
    [[nodiscard]] bool IsAir(BlockID id) const { return id == BLOCK_AIR; }
    
    /**
     * Get tint color for a block face.
     * @param id Block ID
     * @param face Face direction
     * @return Tint color (white = no tint)
     */
    [[nodiscard]] glm::vec3 GetTintColor(BlockID id, Face face) const;

    /**
     * Get the number of registered blocks.
     */
    [[nodiscard]] size_t GetBlockCount() const { return m_Blocks.size(); }

    /**
     * Check if blocks have been loaded.
     */
    [[nodiscard]] bool IsLoaded() const { return !m_Blocks.empty(); }

    /**
     * Clear all block definitions.
     */
    void Clear();

private:
    BlockRegistry() = default;
    ~BlockRegistry() = default;

    // Initialize default air block
    void InitializeAirBlock();

    std::vector<BlockDef> m_Blocks;
    std::unordered_map<std::string, BlockID> m_StringToId;
    
    // Default block for invalid lookups
    BlockDef m_AirBlock;
};

// Helper functions for compatibility with existing code
inline bool IsOpaque(BlockID id) { return BlockRegistry::Instance().IsOpaque(id); }
inline bool IsSolid(BlockID id) { return BlockRegistry::Instance().IsSolid(id); }
inline bool IsTransparent(BlockID id) { return BlockRegistry::Instance().IsTransparent(id); }
inline int GetTextureIndex(BlockID id, Face face) { return BlockRegistry::Instance().GetTextureIndex(id, face); }
inline glm::vec3 GetTintColor(BlockID id, Face face) { return BlockRegistry::Instance().GetTintColor(id, face); }


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

} // namespace Voxel
