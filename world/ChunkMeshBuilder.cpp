#include "ChunkMeshBuilder.h"

namespace Voxel {

ChunkMesh ChunkMeshBuilder::BuildMesh(const Chunk& chunk) {
    ChunkMesh mesh;
    mesh.vertices.reserve(CHUNK_VOLUME * 6);  // Rough estimate, will resize as needed
    mesh.indices.reserve(CHUNK_VOLUME * 6);

    // Iterate through all blocks in the chunk
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                BlockType block = chunk.GetBlock(x, y, z);

                // Skip air blocks - nothing to render
                if (block == BlockType::Air) {
                    continue;
                }

                glm::vec3 position(static_cast<float>(x), 
                                   static_cast<float>(y), 
                                   static_cast<float>(z));

                // Check each face for visibility (face culling)
                // Only add face if neighbor is Air (or out of bounds)
                
                // Top face (+Y)
                if (!IsOpaque(chunk.GetNeighborBlock(x, y, z, Face::Top))) {
                    AddFace(mesh, position, Face::Top, block);
                }

                // Bottom face (-Y)
                if (!IsOpaque(chunk.GetNeighborBlock(x, y, z, Face::Bottom))) {
                    AddFace(mesh, position, Face::Bottom, block);
                }

                // North face (+Z)
                if (!IsOpaque(chunk.GetNeighborBlock(x, y, z, Face::North))) {
                    AddFace(mesh, position, Face::North, block);
                }

                // South face (-Z)
                if (!IsOpaque(chunk.GetNeighborBlock(x, y, z, Face::South))) {
                    AddFace(mesh, position, Face::South, block);
                }

                // East face (+X)
                if (!IsOpaque(chunk.GetNeighborBlock(x, y, z, Face::East))) {
                    AddFace(mesh, position, Face::East, block);
                }

                // West face (-X)
                if (!IsOpaque(chunk.GetNeighborBlock(x, y, z, Face::West))) {
                    AddFace(mesh, position, Face::West, block);
                }
            }
        }
    }

    return mesh;
}

void ChunkMeshBuilder::AddFace(ChunkMesh& mesh, 
                                const glm::vec3& position, 
                                Face face, 
                                BlockType blockType) {
    // Get the 4 vertices for this face
    glm::vec3 vertices[4];
    GetFaceVertices(face, position, vertices);

    // Get UV coordinates
    glm::vec2 uvs[4];
    GetFaceUVs(uvs);

    // Get normal for this face
    glm::vec3 normal = GetFaceNormal(face);

    // Current vertex index before adding new vertices
    unsigned int baseIndex = static_cast<unsigned int>(mesh.vertices.size());

    // Add 4 vertices for the face
    for (int i = 0; i < 4; ++i) {
        mesh.vertices.push_back({vertices[i], uvs[i], normal});
    }

    // Add 2 triangles (6 indices) for the face
    // Triangle 1: 0, 1, 2
    // Triangle 2: 0, 2, 3
    mesh.indices.push_back(baseIndex + 0);
    mesh.indices.push_back(baseIndex + 1);
    mesh.indices.push_back(baseIndex + 2);
    mesh.indices.push_back(baseIndex + 0);
    mesh.indices.push_back(baseIndex + 2);
    mesh.indices.push_back(baseIndex + 3);
}

void ChunkMeshBuilder::GetFaceVertices(Face face, 
                                        const glm::vec3& pos,
                                        glm::vec3 outVertices[4]) {
    // Each face has 4 vertices, defined in counter-clockwise order
    // Block occupies from pos to pos + (1,1,1)
    switch (face) {
        case Face::Top: // +Y face (y = 1)
            outVertices[0] = pos + glm::vec3(0, 1, 0);
            outVertices[1] = pos + glm::vec3(0, 1, 1);
            outVertices[2] = pos + glm::vec3(1, 1, 1);
            outVertices[3] = pos + glm::vec3(1, 1, 0);
            break;

        case Face::Bottom: // -Y face (y = 0)
            outVertices[0] = pos + glm::vec3(0, 0, 1);
            outVertices[1] = pos + glm::vec3(0, 0, 0);
            outVertices[2] = pos + glm::vec3(1, 0, 0);
            outVertices[3] = pos + glm::vec3(1, 0, 1);
            break;

        case Face::North: // +Z face (z = 1)
            outVertices[0] = pos + glm::vec3(1, 0, 1);
            outVertices[1] = pos + glm::vec3(1, 1, 1);
            outVertices[2] = pos + glm::vec3(0, 1, 1);
            outVertices[3] = pos + glm::vec3(0, 0, 1);
            break;

        case Face::South: // -Z face (z = 0)
            outVertices[0] = pos + glm::vec3(0, 0, 0);
            outVertices[1] = pos + glm::vec3(0, 1, 0);
            outVertices[2] = pos + glm::vec3(1, 1, 0);
            outVertices[3] = pos + glm::vec3(1, 0, 0);
            break;

        case Face::East: // +X face (x = 1)
            outVertices[0] = pos + glm::vec3(1, 0, 0);
            outVertices[1] = pos + glm::vec3(1, 1, 0);
            outVertices[2] = pos + glm::vec3(1, 1, 1);
            outVertices[3] = pos + glm::vec3(1, 0, 1);
            break;

        case Face::West: // -X face (x = 0)
            outVertices[0] = pos + glm::vec3(0, 0, 1);
            outVertices[1] = pos + glm::vec3(0, 1, 1);
            outVertices[2] = pos + glm::vec3(0, 1, 0);
            outVertices[3] = pos + glm::vec3(0, 0, 0);
            break;
    }
}

void ChunkMeshBuilder::GetFaceUVs(glm::vec2 outUVs[4]) {
    // Simple 0-1 UV mapping for each face
    // Counter-clockwise from bottom-left
    outUVs[0] = glm::vec2(0.0f, 0.0f);  // Bottom-left
    outUVs[1] = glm::vec2(0.0f, 1.0f);  // Top-left
    outUVs[2] = glm::vec2(1.0f, 1.0f);  // Top-right
    outUVs[3] = glm::vec2(1.0f, 0.0f);  // Bottom-right
}

glm::vec3 ChunkMeshBuilder::GetFaceNormal(Face face) {
    switch (face) {
        case Face::Top:    return glm::vec3( 0,  1,  0);
        case Face::Bottom: return glm::vec3( 0, -1,  0);
        case Face::North:  return glm::vec3( 0,  0,  1);
        case Face::South:  return glm::vec3( 0,  0, -1);
        case Face::East:   return glm::vec3( 1,  0,  0);
        case Face::West:   return glm::vec3(-1,  0,  0);
        default:           return glm::vec3( 0,  1,  0);
    }
}

} // namespace Voxel
