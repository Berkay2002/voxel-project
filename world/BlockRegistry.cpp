#include "BlockRegistry.h"
#include "core/rendering/TextureManager.h"
#include "core/Logger.h"

#include <nlohmann/json.hpp>
#include <fstream>

namespace Voxel {

BlockRegistry& BlockRegistry::Instance() {
    static BlockRegistry instance;
    return instance;
}

void BlockRegistry::InitializeAirBlock() {
    m_AirBlock.id = BLOCK_AIR;
    m_AirBlock.stringId = "air";
    m_AirBlock.solid = false;
    m_AirBlock.opaque = false;
    m_AirBlock.transparent = false;
    m_AirBlock.textureTop = 0;
    m_AirBlock.textureBottom = 0;
    m_AirBlock.textureNorth = 0;
    m_AirBlock.textureSouth = 0;
    m_AirBlock.textureEast = 0;
    m_AirBlock.textureWest = 0;
}

bool BlockRegistry::LoadFromFile(const std::string& path, Core::TextureRegistry& textureRegistry) {
    // Initialize air block first
    InitializeAirBlock();

    // Open and parse JSON file
    std::ifstream file(path);
    if (!file.is_open()) {
        Core::LogError("BlockRegistry: Failed to open file: " + path);
        return false;
    }

    nlohmann::json root;
    try {
        file >> root;
    } catch (const nlohmann::json::parse_error& e) {
        Core::LogError("BlockRegistry: JSON parse error in " + path + ": " + e.what());
        return false;
    }

    // Clear existing blocks
    m_Blocks.clear();
    m_StringToId.clear();

    // Add air block at index 0
    m_Blocks.push_back(m_AirBlock);
    m_StringToId["air"] = BLOCK_AIR;

    // Parse blocks array
    if (!root.contains("blocks") || !root["blocks"].is_array()) {
        Core::LogError("BlockRegistry: No 'blocks' array in " + path);
        return false;
    }

    for (const auto& blockJson : root["blocks"]) {
        // Skip air - already added
        std::string stringId = blockJson.value("id", "");
        if (stringId.empty() || stringId == "air") {
            continue;
        }

        BlockDef def;
        def.id = static_cast<BlockID>(m_Blocks.size());
        def.stringId = stringId;

        // Parse properties
        if (blockJson.contains("properties")) {
            const auto& props = blockJson["properties"];
            def.solid = props.value("solid", true);
            def.opaque = props.value("opaque", true);
            def.transparent = props.value("transparent", false);
            def.tinted = props.value("tinted", false);
            def.tintTop = props.value("tintTop", def.tinted);
            def.tintSides = props.value("tintSides", false);
            
            // Parse tint color if specified
            if (props.contains("tintColor") && props["tintColor"].is_array()) {
                auto arr = props["tintColor"];
                if (arr.size() >= 3) {
                    def.tintColor = glm::vec3(
                        arr[0].get<float>(),
                        arr[1].get<float>(),
                        arr[2].get<float>()
                    );
                }
            }
        }

        // Parse textures
        if (blockJson.contains("textures")) {
            const auto& textures = blockJson["textures"];
            
            // Check for "all" shorthand first
            if (textures.contains("all")) {
                std::string texName = textures["all"].get<std::string>();
                int layerIndex = textureRegistry.GetLayerIndex(texName);
                if (layerIndex < 0) {
                    Core::LogWarn("BlockRegistry: Block '" + stringId + "' uses unknown texture: " + texName);
                    layerIndex = 0; // Fallback to first texture
                }
                def.textureTop = layerIndex;
                def.textureBottom = layerIndex;
                def.textureNorth = layerIndex;
                def.textureSouth = layerIndex;
                def.textureEast = layerIndex;
                def.textureWest = layerIndex;
            }
            
            // Override with specific faces
            auto resolveTexture = [&](const std::string& key) -> int {
                if (textures.contains(key)) {
                    std::string texName = textures[key].get<std::string>();
                    int idx = textureRegistry.GetLayerIndex(texName);
                    return (idx >= 0) ? idx : 0;
                }
                return -1; // Not specified
            };
            
            int top = resolveTexture("top");
            int bottom = resolveTexture("bottom");
            int sides = resolveTexture("sides");
            int north = resolveTexture("north");
            int south = resolveTexture("south");
            int east = resolveTexture("east");
            int west = resolveTexture("west");
            
            if (top >= 0) def.textureTop = top;
            if (bottom >= 0) def.textureBottom = bottom;
            
            // "sides" applies to all 4 horizontal faces
            if (sides >= 0) {
                def.textureNorth = sides;
                def.textureSouth = sides;
                def.textureEast = sides;
                def.textureWest = sides;
            }
            
            // Individual face overrides
            if (north >= 0) def.textureNorth = north;
            if (south >= 0) def.textureSouth = south;
            if (east >= 0) def.textureEast = east;
            if (west >= 0) def.textureWest = west;
        }

        m_StringToId[stringId] = def.id;
        m_Blocks.push_back(def);

        Core::LogDebug("BlockRegistry: Registered block '" + stringId + "' (ID: " + std::to_string(def.id) + ")");
    }

    Core::LogInfo("BlockRegistry: Loaded " + std::to_string(m_Blocks.size()) + " blocks from " + path);
    return true;
}

BlockID BlockRegistry::GetBlockID(const std::string& stringId) const {
    auto it = m_StringToId.find(stringId);
    if (it != m_StringToId.end()) {
        return it->second;
    }
    Core::LogWarn("BlockRegistry: Unknown block: " + stringId);
    return BLOCK_INVALID;
}

const BlockDef& BlockRegistry::GetBlockDef(BlockID id) const {
    if (id < m_Blocks.size()) {
        return m_Blocks[id];
    }
    return m_AirBlock;
}

int BlockRegistry::GetTextureIndex(BlockID id, Face face) const {
    return GetBlockDef(id).GetTextureIndex(face);
}

bool BlockRegistry::IsOpaque(BlockID id) const {
    return GetBlockDef(id).opaque;
}

bool BlockRegistry::IsSolid(BlockID id) const {
    return GetBlockDef(id).solid;
}

bool BlockRegistry::IsTransparent(BlockID id) const {
    return GetBlockDef(id).transparent;
}

void BlockRegistry::Clear() {
    m_Blocks.clear();
    m_StringToId.clear();
}

glm::vec3 BlockRegistry::GetTintColor(BlockID id, Face face) const {
    return GetBlockDef(id).GetFaceTintColor(face);
}

} // namespace Voxel
