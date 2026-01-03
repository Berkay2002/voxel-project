#pragma once

#include "Chunk.h"
#include "TerrainGenerator.h"
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace Core {
class Shader;
class Camera;
}

namespace Voxel {

// Chunk coordinate for map key
struct ChunkCoord {
    int x, z;
    
    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
};

// Hash function for ChunkCoord
struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& coord) const {
        // Combine x and z into a single hash
        return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.z) << 16);
    }
};

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager() = default;

    // Update chunks based on player position (load/unload)
    void Update(const glm::vec3& playerPos);

    // Render all loaded chunks
    void RenderAll(Core::Shader& shader, Core::Camera& camera, float aspectRatio);

    // Get chunk at given chunk coordinates (nullptr if not loaded)
    Chunk* GetChunk(int chunkX, int chunkZ);

    // Get number of loaded chunks
    size_t GetLoadedChunkCount() const { return m_Chunks.size(); }

    // Configuration
    void SetLoadRadius(int radius) { m_LoadRadius = radius; }
    int GetLoadRadius() const { return m_LoadRadius; }

private:
    // Convert world position to chunk coordinates
    ChunkCoord WorldToChunkCoord(const glm::vec3& worldPos) const;

    // Load a chunk at the given coordinates
    void LoadChunk(int chunkX, int chunkZ);

    // Unload a chunk at the given coordinates
    void UnloadChunk(int chunkX, int chunkZ);

    // Check if a chunk should be loaded based on distance
    bool ShouldBeLoaded(int chunkX, int chunkZ, const ChunkCoord& center) const;

    // Chunk storage
    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_Chunks;

    // Terrain generator
    TerrainGenerator m_TerrainGenerator;

    // Load/unload configuration
    int m_LoadRadius = 2;      // Chunks to load around player (5x5 grid with radius 2)
    int m_UnloadRadius = 3;    // Chunks beyond this are unloaded (hysteresis)

    // Current center chunk (to detect when player moves to new chunk)
    ChunkCoord m_CenterChunk = {0, 0};
};

} // namespace Voxel
