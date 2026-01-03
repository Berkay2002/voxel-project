#pragma once

#include "Block.h"
#include <array>

namespace Voxel {

// Chunk dimensions (Minecraft-style)
constexpr int CHUNK_WIDTH = 16;   // X
constexpr int CHUNK_HEIGHT = 256; // Y
constexpr int CHUNK_DEPTH = 16;   // Z
constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

class Chunk {
public:
    Chunk();
    ~Chunk() = default;

    // Block access
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
};

} // namespace Voxel
