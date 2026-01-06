#pragma once

#include "Block.h"
#include "ChunkTask.h"
#include <array>
#include <atomic>

// Forward declarations
namespace Core {
class VertexArray;
class VertexBuffer;
class IndexBuffer;
}

namespace Voxel {

// Chunk dimensions (Minecraft-style)
constexpr int CHUNK_WIDTH = 16;   // X
constexpr int CHUNK_HEIGHT = 256; // Y
constexpr int CHUNK_DEPTH = 16;   // Z
constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

class Chunk {
public:
    Chunk();
    ~Chunk();

    // Block access (thread-safe for reading during mesh generation)
    BlockType GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockType type);

    // Bounds checking
    bool IsInBounds(int x, int y, int z) const;

    // Get neighbor block (returns Air if out of bounds)
    BlockType GetNeighborBlock(int x, int y, int z, Face face) const;

    // Check if the chunk needs mesh rebuild
    bool IsDirty() const { return m_Dirty; }
    void SetDirty(bool dirty) { m_Dirty = dirty; }

    // Chunk world position (in chunk coordinates, not block coordinates)
    void SetPosition(int chunkX, int chunkZ) { m_ChunkX = chunkX; m_ChunkZ = chunkZ; }
    int GetChunkX() const { return m_ChunkX; }
    int GetChunkZ() const { return m_ChunkZ; }

    // Thread-safe state management
    ChunkState GetState() const { return m_State.load(std::memory_order_acquire); }
    void SetState(ChunkState state) { m_State.store(state, std::memory_order_release); }

    // Mesh management - SYNCHRONOUS (original method, for backwards compatibility)
    void BuildMesh();
    void UploadMesh();
    void CleanupMesh();
    bool HasMesh() const { return m_HasMesh; }
    
    // Mesh management - ASYNC (thread-safe)
    // Generate mesh data without OpenGL calls (safe for background threads)
    ChunkMeshData GenerateMeshData() const;
    // Upload pre-generated mesh data to GPU (main thread only!)
    void UploadMeshFromData(const ChunkMeshData& data);
    
    // Render this chunk's mesh (opaque geometry)
    void Render() const;
    unsigned int GetIndexCount() const { return m_IndexCount; }
    
    // Render this chunk's water mesh (transparent geometry)
    void RenderWater() const;
    bool HasWaterMesh() const { return m_HasWaterMesh; }
    unsigned int GetWaterIndexCount() const { return m_WaterIndexCount; }

    // Heightmap for weather occlusion (highest solid block at each XZ column)
    // Returns the Y coordinate of the highest solid block, or -1 if entire column is air
    int GetHeightAt(int x, int z) const;
    
    // Rebuild the heightmap (called after terrain generation or block changes)
    void RebuildHeightmap();

private:
    // Convert 3D coordinates to 1D array index
    int GetIndex(int x, int y, int z) const;

    // Block data stored as 1D array for cache efficiency
    std::array<BlockType, CHUNK_VOLUME> m_Blocks;

    // Chunk position in world
    int m_ChunkX = 0;
    int m_ChunkZ = 0;

    // Dirty flag for mesh rebuilding
    bool m_Dirty = true;

    // Thread-safe state for async loading
    std::atomic<ChunkState> m_State{ChunkState::Unloaded};

    // GPU mesh resources for opaque geometry (owned by chunk)
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_IBO = 0;
    unsigned int m_IndexCount = 0;
    bool m_HasMesh = false;
    
    // GPU mesh resources for water (transparent geometry)
    unsigned int m_WaterVAO = 0;
    unsigned int m_WaterVBO = 0;
    unsigned int m_WaterIBO = 0;
    unsigned int m_WaterIndexCount = 0;
    bool m_HasWaterMesh = false;
    
    // Heightmap for weather particles (highest solid block per XZ column)
    // 16x16 array storing Y coordinates (-1 = no blocks in column)
    std::array<int, CHUNK_WIDTH * CHUNK_DEPTH> m_Heightmap;
};

} // namespace Voxel
