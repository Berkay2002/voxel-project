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
    float ao;  // Ambient occlusion: 0.0 (fully occluded) to 1.0 (fully lit)
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
    // Add a single face to the mesh (with AO calculation)
    void AddFace(ChunkMesh& mesh, 
                 const Chunk& chunk,
                 int x, int y, int z,
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

    // Calculate ambient occlusion for each vertex of a face
    // Returns 4 AO values (one per vertex), range 0.0-1.0
    void CalculateFaceAO(const Chunk& chunk, int x, int y, int z, 
                         Face face, float outAO[4]);

    // Calculate AO for a single vertex based on 3 neighbor checks
    float CalculateVertexAO(bool side1, bool side2, bool corner);

    // Check if a block at position is opaque (for AO calculation)
    bool IsBlockOpaque(const Chunk& chunk, int x, int y, int z);
};

} // namespace Voxel
