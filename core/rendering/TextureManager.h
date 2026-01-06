#pragma once

#include "../graphics/TextureArray.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Core {

/**
 * TextureManager manages texture name to layer index mapping for the texture array.
 * It provides a centralized way to load textures and look up layer indices by name.
 * 
 * Renamed from TextureRegistry to match AAA engine conventions.
 * 
 * Usage:
 *   TextureManager& manager = TextureManager::Instance();
 *   manager.LoadTextures({"grass_block_top", "dirt", "stone"}, "assets/textures/blocks/");
 *   int layer = manager.GetLayerIndex("dirt"); // Returns 1
 */
class TextureManager {
public:
    // Singleton access
    static TextureManager& Instance();

    // Delete copy/move for singleton
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    /**
     * Load textures from a list of texture names.
     * Each name should be without the .png extension.
     * @param textureNames List of texture names (e.g., "grass_block_top", "dirt")
     * @param directory Base directory for textures (e.g., "assets/textures/blocks/")
     * @return true if all textures loaded successfully
     */
    bool LoadTextures(const std::vector<std::string>& textureNames,
                      const std::string& directory = "assets/textures/blocks/");

    /**
     * Get the layer index for a texture name.
     * @param textureName Texture name without .png extension
     * @return Layer index, or -1 if not found
     */
    [[nodiscard]] int GetLayerIndex(const std::string& textureName) const;

    /**
     * Check if a texture is registered.
     */
    [[nodiscard]] bool HasTexture(const std::string& textureName) const;

    /**
     * Get the underlying texture array for binding.
     * @return Reference to the texture array, or nullptr if not loaded
     */
    [[nodiscard]] TextureArray* GetTextureArray() const { return m_TextureArray.get(); }

    /**
     * Check if textures have been loaded.
     */
    [[nodiscard]] bool IsLoaded() const { return m_TextureArray != nullptr && m_TextureArray->IsValid(); }

    /**
     * Get the number of loaded textures.
     */
    [[nodiscard]] int GetTextureCount() const { return static_cast<int>(m_NameToLayer.size()); }

    /**
     * Clear all loaded textures.
     */
    void Clear();

private:
    TextureManager() = default;
    ~TextureManager() = default;

    std::unordered_map<std::string, int> m_NameToLayer;
    std::unique_ptr<TextureArray> m_TextureArray;
};

// Backwards compatibility alias (can be removed later)
using TextureRegistry = TextureManager;

} // namespace Core
