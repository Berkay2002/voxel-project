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
        // Resolve string identifier (stringId preferred, fall back to id which may be string or number)
        std::string stringId;
        if (blockJson.contains("stringId") && blockJson["stringId"].is_string()) {
            stringId = blockJson["stringId"].get<std::string>();
        } else if (blockJson.contains("id")) {
            const auto& idField = blockJson["id"];
            if (idField.is_string()) {
                stringId = idField.get<std::string>();
            } else if (idField.is_number_integer()) {
                stringId = std::to_string(idField.get<int>());
            }
        }

        // Skip invalid or duplicate air entries (air already injected at index 0)
        if (stringId.empty() || stringId == "air") {
            continue;
        }

        BlockDef def;
        def.id = static_cast<BlockID>(m_Blocks.size());
        def.stringId = stringId;

        // Helpers to support both top-level and nested property blocks
        auto getBoolProp = [&](const char* key, bool defaultVal) {
            if (blockJson.contains(key) && blockJson[key].is_boolean()) {
                return blockJson[key].get<bool>();
            }
            if (blockJson.contains("properties")) {
                const auto& props = blockJson["properties"];
                if (props.contains(key) && props[key].is_boolean()) {
                    return props[key].get<bool>();
                }
            }
            return defaultVal;
        };

        auto applyTint = [&](glm::vec3& tint) {
            bool found = false;
            const nlohmann::json* tintArray = nullptr;
            if (blockJson.contains("tintColor") && blockJson["tintColor"].is_array()) {
                tintArray = &blockJson["tintColor"];
            } else if (blockJson.contains("properties")) {
                const auto& props = blockJson["properties"];
                if (props.contains("tintColor") && props["tintColor"].is_array()) {
                    tintArray = &props["tintColor"];
                }
            }

            if (tintArray && tintArray->size() >= 3) {
                tint = glm::vec3(
                    (*tintArray)[0].get<float>(),
                    (*tintArray)[1].get<float>(),
                    (*tintArray)[2].get<float>()
                );
                found = true;
            }
            return found;
        };

        // Parse properties with fallbacks
        def.solid = getBoolProp("solid", def.solid);
        def.opaque = getBoolProp("opaque", def.opaque);
        def.transparent = getBoolProp("transparent", def.transparent);
        def.tinted = getBoolProp("tinted", def.tinted);
        def.tintTop = getBoolProp("tintTop", def.tinted);
        def.tintSides = getBoolProp("tintSides", def.tintSides);
        bool tintSpecified = applyTint(def.tintColor);
        if (tintSpecified && !def.tinted) {
            // If a tint color is provided without explicit flags, tint all faces by default
            def.tinted = true;
            def.tintTop = true;
            def.tintSides = true;
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
    // Fallback for cases where registry has not been loaded (e.g., unit tests)
    if (m_Blocks.empty()) {
        return id != BLOCK_AIR;
    }
    return GetBlockDef(id).solid;
}

bool BlockRegistry::IsTransparent(BlockID id) const {
    return GetBlockDef(id).transparent;
}

void BlockRegistry::Clear() {
    m_Blocks.clear();
    m_StringToId.clear();
    InitializeAirBlock();
}

glm::vec3 BlockRegistry::GetTintColor(BlockID id, Face face) const {
    return GetBlockDef(id).GetFaceTintColor(face);
}

} // namespace Voxel
