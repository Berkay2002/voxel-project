#include "Chunk.h"

namespace Voxel {

Chunk::Chunk() {
    // Initialize all blocks to Air
    m_Blocks.fill(BlockType::Air);
}

int Chunk::GetIndex(int x, int y, int z) const {
    // Y-major ordering for vertical column access patterns
    // Index = y * (WIDTH * DEPTH) + z * WIDTH + x
    return y * (CHUNK_WIDTH * CHUNK_DEPTH) + z * CHUNK_WIDTH + x;
}

bool Chunk::IsInBounds(int x, int y, int z) const {
    return x >= 0 && x < CHUNK_WIDTH &&
           y >= 0 && y < CHUNK_HEIGHT &&
           z >= 0 && z < CHUNK_DEPTH;
}

BlockType Chunk::GetBlock(int x, int y, int z) const {
    if (!IsInBounds(x, y, z)) {
        return BlockType::Air;
    }
    return m_Blocks[GetIndex(x, y, z)];
}

void Chunk::SetBlock(int x, int y, int z, BlockType type) {
    if (!IsInBounds(x, y, z)) {
        return;
    }
    m_Blocks[GetIndex(x, y, z)] = type;
    m_Dirty = true;
}

BlockType Chunk::GetNeighborBlock(int x, int y, int z, Face face) const {
    glm::ivec3 dir = GetFaceDirection(face);
    int nx = x + dir.x;
    int ny = y + dir.y;
    int nz = z + dir.z;

    // Out of bounds neighbors are treated as Air (so the face IS rendered)
    if (!IsInBounds(nx, ny, nz)) {
        return BlockType::Air;
    }

    return m_Blocks[GetIndex(nx, ny, nz)];
}

} // namespace Voxel
