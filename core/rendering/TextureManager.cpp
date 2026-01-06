#include "TextureManager.h"
#include "../Logger.h"

namespace Core {

TextureManager& TextureManager::Instance() {
    static TextureManager instance;
    return instance;
}

bool TextureManager::LoadTextures(const std::vector<std::string>& textureNames,
                                   const std::string& directory) {
    if (textureNames.empty()) {
        LogError("TextureManager: No texture names provided");
        return false;
    }

    // Build full paths and populate name-to-layer mapping
    std::vector<std::string> paths;
    paths.reserve(textureNames.size());
    m_NameToLayer.clear();

    for (size_t i = 0; i < textureNames.size(); ++i) {
        const std::string& name = textureNames[i];
        std::string path = directory + name + ".png";
        paths.push_back(path);
        m_NameToLayer[name] = static_cast<int>(i);
    }

    // Create texture array from paths
    m_TextureArray = std::make_unique<TextureArray>(paths);

    if (!m_TextureArray->IsValid()) {
        LogError("TextureManager: Failed to create texture array");
        m_NameToLayer.clear();
        m_TextureArray.reset();
        return false;
    }

    LogInfo("TextureManager: Loaded " + std::to_string(m_NameToLayer.size()) + " textures");
    return true;
}

int TextureManager::GetLayerIndex(const std::string& textureName) const {
    auto it = m_NameToLayer.find(textureName);
    if (it != m_NameToLayer.end()) {
        return it->second;
    }
    
    // Log warning for missing textures (helps debug block config issues)
    LogWarn("TextureManager: Texture not found: " + textureName);
    return -1;
}

bool TextureManager::HasTexture(const std::string& textureName) const {
    return m_NameToLayer.find(textureName) != m_NameToLayer.end();
}

void TextureManager::Clear() {
    m_NameToLayer.clear();
    m_TextureArray.reset();
}

} // namespace Core
