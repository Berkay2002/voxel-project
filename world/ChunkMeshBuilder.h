#pragma once

#include "Chunk.h"
#include <vector>
#include <glm/glm.hpp>

namespace Voxel {

// Vertex structure for chunk mesh
struct ChunkVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

// Result of mesh building
struct ChunkMesh {
    std::vector<ChunkVertex> vertices;
    std::vector<unsigned int> indices;
    
    void Clear() {
        vertices.clear();
        indices.clear();
    }
    
    bool IsEmpty() const {
        return vertices.empty();
    }
};

class ChunkMeshBuilder {
public:
    ChunkMeshBuilder() = default;
    ~ChunkMeshBuilder() = default;

    // Build mesh from chunk data with face culling
    ChunkMesh BuildMesh(const Chunk& chunk);

private:
    // Add a single face to the mesh
    void AddFace(ChunkMesh& mesh, 
                 const glm::vec3& position, 
                 Face face, 
                 BlockType blockType);

    // Get the 4 vertices for a face
    void GetFaceVertices(Face face, 
                         const glm::vec3& position,
                         glm::vec3 outVertices[4]);

    // Get UV coordinates for a face (full 0-1 range for now)
    void GetFaceUVs(glm::vec2 outUVs[4]);

    // Get normal vector for a face
    glm::vec3 GetFaceNormal(Face face);
};

} // namespace Voxel
