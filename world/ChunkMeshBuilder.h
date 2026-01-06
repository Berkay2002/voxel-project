#pragma once

#include "Chunk.h"
#include "ChunkTask.h"  // For ChunkVertex definition
#include <vector>
#include <glm/glm.hpp>

namespace Voxel {

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

// Combined result with separate opaque and water meshes
struct ChunkMeshResult {
    ChunkMesh opaqueMesh;   // Solid blocks (rendered first, no blending)
    ChunkMesh waterMesh;    // Transparent water blocks (rendered second, with alpha blending)
};

class ChunkMeshBuilder {
public:
    ChunkMeshBuilder() = default;
    ~ChunkMeshBuilder() = default;

    // Build mesh from chunk data with face culling
    // Returns separate opaque and water meshes for proper render ordering
    ChunkMeshResult BuildMesh(const Chunk& chunk);

private:
    // Add a single face to the mesh (with AO calculation)
    void AddFace(ChunkMesh& mesh, 
                 const Chunk& chunk,
                 int x, int y, int z,
                 Face face, 
                 BlockType blockType);

    // Get the 4 vertices for a face
    static void GetFaceVertices(Face face, 
                         const glm::vec3& position,
                         glm::vec3 outVertices[4]);

    // Get UV coordinates for a face (full 0-1 range for now)
    static void GetFaceUVs(glm::vec2 outUVs[4]);

    // Get normal vector for a face
    static glm::vec3 GetFaceNormal(Face face);

    // Calculate ambient occlusion for each vertex of a face
    // Returns 4 AO values (one per vertex), range 0.0-1.0
    void CalculateFaceAO(const Chunk& chunk, int x, int y, int z, 
                         Face face, float outAO[4]);

    // Calculate AO for a single vertex based on 3 neighbor checks
    static float CalculateVertexAO(bool side1, bool side2, bool corner);

    // Check if a block at position is opaque (for AO calculation)
    static bool IsBlockOpaque(const Chunk& chunk, int x, int y, int z);

    // Compute heightmap for sky light calculation (highest solid block per column)
    void ComputeHeightMap(const Chunk& chunk);

    // Heightmap: highest solid block Y for each (x, z) column
    // Blocks at Y > heightMap[x][z] have sky access
    int m_HeightMap[CHUNK_WIDTH][CHUNK_DEPTH];
};

} // namespace Voxel
