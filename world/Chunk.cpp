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
    ChunkMeshResult result = builder.BuildMesh(*this);
    
    // Upload opaque mesh
    if (result.opaqueMesh.IsEmpty()) {
        // Delete opaque buffers if we had them before
        if (m_VAO != 0) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_IBO);
            m_VAO = m_VBO = m_IBO = 0;
        }
        m_HasMesh = false;
        m_IndexCount = 0;
    } else {
        // Create OpenGL buffers if they don't exist
        if (m_VAO == 0) {
            glGenVertexArrays(1, &m_VAO);
            glGenBuffers(1, &m_VBO);
            glGenBuffers(1, &m_IBO);
        }

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, 
                     result.opaqueMesh.vertices.size() * sizeof(ChunkVertex),
                     result.opaqueMesh.vertices.data(), 
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

        // AO attribute (location 3)
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                              (void*)offsetof(ChunkVertex, ao));
        glEnableVertexAttribArray(3);

        // TexIndex attribute (location 4)
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                              (void*)offsetof(ChunkVertex, texIndex));
        glEnableVertexAttribArray(4);

        // TintColor attribute (location 5)
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                              (void*)offsetof(ChunkVertex, tintColor));
        glEnableVertexAttribArray(5);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     result.opaqueMesh.indices.size() * sizeof(unsigned int),
                     result.opaqueMesh.indices.data(),
                     GL_STATIC_DRAW);

        glBindVertexArray(0);

        m_IndexCount = static_cast<unsigned int>(result.opaqueMesh.indices.size());
        m_HasMesh = true;
    }
    
    // Upload water mesh
    if (result.waterMesh.IsEmpty()) {
        // Delete water buffers if we had them before
        if (m_WaterVAO != 0) {
            glDeleteVertexArrays(1, &m_WaterVAO);
            glDeleteBuffers(1, &m_WaterVBO);
            glDeleteBuffers(1, &m_WaterIBO);
            m_WaterVAO = m_WaterVBO = m_WaterIBO = 0;
        }
        m_HasWaterMesh = false;
        m_WaterIndexCount = 0;
    } else {
        // Create OpenGL buffers if they don't exist
        if (m_WaterVAO == 0) {
            glGenVertexArrays(1, &m_WaterVAO);
            glGenBuffers(1, &m_WaterVBO);
            glGenBuffers(1, &m_WaterIBO);
        }

        glBindVertexArray(m_WaterVAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_WaterVBO);
        glBufferData(GL_ARRAY_BUFFER, 
                     result.waterMesh.vertices.size() * sizeof(ChunkVertex),
                     result.waterMesh.vertices.data(), 
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

        // AO attribute (location 3)
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                              (void*)offsetof(ChunkVertex, ao));
        glEnableVertexAttribArray(3);

        // TexIndex attribute (location 4)
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                              (void*)offsetof(ChunkVertex, texIndex));
        glEnableVertexAttribArray(4);

        // TintColor attribute (location 5)
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), 
                              (void*)offsetof(ChunkVertex, tintColor));
        glEnableVertexAttribArray(5);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_WaterIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     result.waterMesh.indices.size() * sizeof(unsigned int),
                     result.waterMesh.indices.data(),
                     GL_STATIC_DRAW);

        glBindVertexArray(0);

        m_WaterIndexCount = static_cast<unsigned int>(result.waterMesh.indices.size());
        m_HasWaterMesh = true;
    }
    
    m_Dirty = false;
}

void Chunk::UploadMesh() {
    // Alias for BuildMesh - rebuilds and uploads
    BuildMesh();
}

void Chunk::CleanupMesh() {
    // Cleanup opaque mesh
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
    
    // Cleanup water mesh
    if (m_WaterVAO != 0) {
        glDeleteVertexArrays(1, &m_WaterVAO);
        m_WaterVAO = 0;
    }
    if (m_WaterVBO != 0) {
        glDeleteBuffers(1, &m_WaterVBO);
        m_WaterVBO = 0;
    }
    if (m_WaterIBO != 0) {
        glDeleteBuffers(1, &m_WaterIBO);
        m_WaterIBO = 0;
    }
    m_WaterIndexCount = 0;
    m_HasWaterMesh = false;
}

ChunkMeshData Chunk::GenerateMeshData() const {
    // This method is thread-safe - NO OpenGL calls!
    // Build mesh using the mesh builder
    ChunkMeshBuilder builder;
    ChunkMeshResult result = builder.BuildMesh(*this);
    
    ChunkMeshData data;
    data.chunkX = m_ChunkX;
    data.chunkZ = m_ChunkZ;
    data.valid = true;
    
    // Convert opaque mesh ChunkVertex array to flat float array for GPU upload
    // Layout: position (3) + uv (2) + normal (3) + ao (1) + texIndex (1) + tintColor (3) = 13 floats per vertex
    if (!result.opaqueMesh.IsEmpty()) {
        data.opaqueVertices.reserve(result.opaqueMesh.vertices.size() * 13);
        for (const auto& vertex : result.opaqueMesh.vertices) {
            data.opaqueVertices.push_back(vertex.position.x);
            data.opaqueVertices.push_back(vertex.position.y);
            data.opaqueVertices.push_back(vertex.position.z);
            data.opaqueVertices.push_back(vertex.uv.x);
            data.opaqueVertices.push_back(vertex.uv.y);
            data.opaqueVertices.push_back(vertex.normal.x);
            data.opaqueVertices.push_back(vertex.normal.y);
            data.opaqueVertices.push_back(vertex.normal.z);
            data.opaqueVertices.push_back(vertex.ao);
            data.opaqueVertices.push_back(vertex.texIndex);
            data.opaqueVertices.push_back(vertex.tintColor.r);
            data.opaqueVertices.push_back(vertex.tintColor.g);
            data.opaqueVertices.push_back(vertex.tintColor.b);
        }
        data.opaqueIndices = std::move(result.opaqueMesh.indices);
    }
    
    // Convert water mesh to flat float array
    if (!result.waterMesh.IsEmpty()) {
        data.waterVertices.reserve(result.waterMesh.vertices.size() * 13);
        for (const auto& vertex : result.waterMesh.vertices) {
            data.waterVertices.push_back(vertex.position.x);
            data.waterVertices.push_back(vertex.position.y);
            data.waterVertices.push_back(vertex.position.z);
            data.waterVertices.push_back(vertex.uv.x);
            data.waterVertices.push_back(vertex.uv.y);
            data.waterVertices.push_back(vertex.normal.x);
            data.waterVertices.push_back(vertex.normal.y);
            data.waterVertices.push_back(vertex.normal.z);
            data.waterVertices.push_back(vertex.ao);
            data.waterVertices.push_back(vertex.texIndex);
            data.waterVertices.push_back(vertex.tintColor.r);
            data.waterVertices.push_back(vertex.tintColor.g);
            data.waterVertices.push_back(vertex.tintColor.b);
        }
        data.waterIndices = std::move(result.waterMesh.indices);
    }
    
    return data;
}

void Chunk::UploadMeshFromData(const ChunkMeshData& data) {
    // MAIN THREAD ONLY - makes OpenGL calls!
    
    if (!data.valid) {
        CleanupMesh();
        return;
    }
    
    // Vertex layout: position (3) + uv (2) + normal (3) + ao (1) + texIndex (1) + tintColor (3) = 13 floats = 52 bytes stride
    constexpr GLsizei stride = 13 * sizeof(float);
    
    // Upload opaque mesh
    if (data.opaqueVertices.empty()) {
        // Delete opaque buffers if we had them before
        if (m_VAO != 0) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_IBO);
            m_VAO = m_VBO = m_IBO = 0;
        }
        m_HasMesh = false;
        m_IndexCount = 0;
    } else {
        // Create OpenGL buffers if they don't exist
        if (m_VAO == 0) {
            glGenVertexArrays(1, &m_VAO);
            glGenBuffers(1, &m_VBO);
            glGenBuffers(1, &m_IBO);
        }
        
        glBindVertexArray(m_VAO);
        
        // Upload vertex data (flat float array)
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, 
                     data.opaqueVertices.size() * sizeof(float),
                     data.opaqueVertices.data(), 
                     GL_STATIC_DRAW);
        
        // Position attribute (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);
        
        // UV attribute (location 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Normal attribute (location 2)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        // AO attribute (location 3)
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);
        
        // TexIndex attribute (location 4)
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(4);
        
        // TintColor attribute (location 5)
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, stride, (void*)(10 * sizeof(float)));
        glEnableVertexAttribArray(5);
        
        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     data.opaqueIndices.size() * sizeof(unsigned int),
                     data.opaqueIndices.data(),
                     GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        
        m_IndexCount = static_cast<unsigned int>(data.opaqueIndices.size());
        m_HasMesh = true;
    }
    
    // Upload water mesh
    if (data.waterVertices.empty()) {
        // Delete water buffers if we had them before
        if (m_WaterVAO != 0) {
            glDeleteVertexArrays(1, &m_WaterVAO);
            glDeleteBuffers(1, &m_WaterVBO);
            glDeleteBuffers(1, &m_WaterIBO);
            m_WaterVAO = m_WaterVBO = m_WaterIBO = 0;
        }
        m_HasWaterMesh = false;
        m_WaterIndexCount = 0;
    } else {
        // Create OpenGL buffers if they don't exist
        if (m_WaterVAO == 0) {
            glGenVertexArrays(1, &m_WaterVAO);
            glGenBuffers(1, &m_WaterVBO);
            glGenBuffers(1, &m_WaterIBO);
        }
        
        glBindVertexArray(m_WaterVAO);
        
        // Upload vertex data (flat float array)
        glBindBuffer(GL_ARRAY_BUFFER, m_WaterVBO);
        glBufferData(GL_ARRAY_BUFFER, 
                     data.waterVertices.size() * sizeof(float),
                     data.waterVertices.data(), 
                     GL_STATIC_DRAW);
        
        // Position attribute (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);
        
        // UV attribute (location 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Normal attribute (location 2)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        // AO attribute (location 3)
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);
        
        // TexIndex attribute (location 4)
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(4);
        
        // TintColor attribute (location 5)
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, stride, (void*)(10 * sizeof(float)));
        glEnableVertexAttribArray(5);
        
        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_WaterIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     data.waterIndices.size() * sizeof(unsigned int),
                     data.waterIndices.data(),
                     GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        
        m_WaterIndexCount = static_cast<unsigned int>(data.waterIndices.size());
        m_HasWaterMesh = true;
    }
    
    m_Dirty = false;
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

void Chunk::RenderWater() const {
    if (!m_HasWaterMesh || m_WaterVAO == 0) {
        return;
    }

    glBindVertexArray(m_WaterVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_WaterIndexCount), 
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace Voxel
