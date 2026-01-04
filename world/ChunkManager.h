#pragma once

#include "Chunk.h"
#include "ChunkTask.h"
#include "TerrainGenerator.h"
#include "WorldConfig.h"
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include <glm/glm.hpp>
#include <BS_thread_pool.hpp>

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
    ~ChunkManager();

    // Update chunks based on player position (load/unload)
    void Update(const glm::vec3& playerPos);

    // Process completed mesh tasks (call from main thread each frame)
    void ProcessPendingMeshes();

    // Render all loaded chunks (opaque geometry pass)
    void RenderAll(Core::Shader& shader, Core::Camera& camera, float aspectRatio);
    
    // Render water for all loaded chunks (transparent pass - call AFTER RenderAll)
    void RenderWater(Core::Shader& waterShader, Core::Camera& camera, float aspectRatio);

    // Get chunk at given chunk coordinates (nullptr if not loaded)
    Chunk* GetChunk(int chunkX, int chunkZ);

    // Get number of loaded chunks
    size_t GetLoadedChunkCount() const { return m_Chunks.size(); }

    // Configuration
    void SetLoadRadius(int radius) { m_LoadRadius = radius; }
    int GetLoadRadius() const { return m_LoadRadius; }

    // Block access at world coordinates (for raycasting and interaction)
    // Returns BLOCK_AIR if position is out of bounds or chunk not loaded
    BlockID GetBlock(int worldX, int worldY, int worldZ) const;
    
    // Set a block at world coordinates, triggers mesh rebuild
    // Does nothing if position is out of bounds or chunk not loaded
    void SetBlock(int worldX, int worldY, int worldZ, BlockID block);

private:
    // Convert world position to chunk coordinates
    ChunkCoord WorldToChunkCoord(const glm::vec3& worldPos) const;

    // Load a chunk synchronously (legacy method)
    void LoadChunk(int chunkX, int chunkZ);

    // Load a chunk asynchronously using thread pool
    void LoadChunkAsync(int chunkX, int chunkZ);

    // Unload a chunk at the given coordinates
    void UnloadChunk(int chunkX, int chunkZ);

    // Check if a chunk should be loaded based on distance
    bool ShouldBeLoaded(int chunkX, int chunkZ, const ChunkCoord& center) const;

    // Rebuild a chunk's mesh asynchronously (used after block changes)
    void RebuildChunkMesh(int chunkX, int chunkZ);

    // Const version of GetChunk for raycasting
    const Chunk* GetChunkConst(int chunkX, int chunkZ) const;

    // Chunk storage
    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_Chunks;

    // Terrain generator (thread-safe for reading config)
    TerrainGenerator m_TerrainGenerator;

    // Thread pool for background chunk work (light version - no priority queue)
    BS::light_thread_pool m_ThreadPool;

    // Thread-safe queue for completed mesh data
    std::queue<ChunkMeshData> m_PendingMeshes;
    std::mutex m_PendingMeshMutex;

    // Load/unload configuration (values from WorldConfig.h)
    int m_LoadRadius = Config::CHUNK_LOAD_RADIUS;
    int m_UnloadRadius = Config::CHUNK_UNLOAD_RADIUS;

    // Current center chunk (to detect when player moves to new chunk)
    ChunkCoord m_CenterChunk = {0, 0};

    // Rate limiting for mesh uploads
    static constexpr int MAX_UPLOADS_PER_FRAME = Config::MAX_MESH_UPLOADS_PER_FRAME;
};

} // namespace Voxel
