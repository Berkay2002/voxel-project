#include "Chunk.h"
#include "ChunkMeshBuilder.h"
#include <glad/gl.h>

namespace Voxel {

Chunk::Chunk() {
    // Initialize all blocks to Air
    m_Blocks.fill(BlockType::Air);
}

Chunk::~Chunk() {
    CleanupMesh();
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

void Chunk::BuildMesh() {
    ChunkMeshBuilder builder;
    ChunkMesh mesh = builder.BuildMesh(*this);
    
    if (mesh.IsEmpty()) {
        CleanupMesh();
        return;
    }

    // Create OpenGL buffers if they don't exist
    if (m_VAO == 0) {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_IBO);
    }

    // Upload vertex data
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 mesh.vertices.size() * sizeof(ChunkVertex),
                 mesh.vertices.data(), 
                 GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                          (void*)offsetof(ChunkVertex, position));
    glEnableVertexAttribArray(0);

    // UV attribute (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                          (void*)offsetof(ChunkVertex, uv));
    glEnableVertexAttribArray(1);

    // Normal attribute (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                          (void*)offsetof(ChunkVertex, normal));
    glEnableVertexAttribArray(2);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);

    m_IndexCount = static_cast<unsigned int>(mesh.indices.size());
    m_HasMesh = true;
    m_Dirty = false;
}

void Chunk::UploadMesh() {
    // Alias for BuildMesh - rebuilds and uploads
    BuildMesh();
}

void Chunk::CleanupMesh() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_IBO != 0) {
        glDeleteBuffers(1, &m_IBO);
        m_IBO = 0;
    }
    m_IndexCount = 0;
    m_HasMesh = false;
}

void Chunk::Render() const {
    if (!m_HasMesh || m_VAO == 0) {
        return;
    }

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_IndexCount), 
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace Voxel
