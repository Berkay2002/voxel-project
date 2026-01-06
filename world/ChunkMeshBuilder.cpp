#include "ChunkMeshBuilder.h"

namespace Voxel {

ChunkMeshResult ChunkMeshBuilder::BuildMesh(const Chunk& chunk) {
    ChunkMeshResult result;
    // Reserve conservative estimates to reduce reallocations
    // A fully generated chunk might have ~24k faces, but with culling it's usually ~3-5k
    result.opaqueMesh.vertices.reserve(4000 * 4);  // 4 vertices per face
    result.opaqueMesh.indices.reserve(4000 * 6);   // 6 indices per face
    result.waterMesh.vertices.reserve(500 * 4);    // Water is less common
    result.waterMesh.indices.reserve(500 * 6);

    // Compute heightmap for sky light calculation (once per chunk)
    ComputeHeightMap(chunk);

    // Iterate through all blocks in the chunk
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                BlockType block = chunk.GetBlock(x, y, z);

                // Skip air blocks - nothing to render
                if (block == BlockType::Air) {
                    continue;
                }

                // Determine which mesh to add faces to
                bool isWater = IsTransparent(block);
                ChunkMesh& targetMesh = isWater ? result.waterMesh : result.opaqueMesh;

                // Check each face for visibility (face culling)
                // For opaque blocks: only add face if neighbor is NOT opaque
                // For water blocks: only add TOP face if neighbor is Air
                //                   Skip side/bottom faces at chunk boundaries (assume water continues)
                
                // Top face (+Y) - always render for water if air above
                BlockType neighborTop = chunk.GetNeighborBlock(x, y, z, Face::Top);
                if (isWater ? (neighborTop == BlockType::Air) : !IsOpaque(neighborTop)) {
                    AddFace(targetMesh, chunk, x, y, z, Face::Top, block);
                }

                // Bottom face (-Y) - Skip for water entirely
                BlockType neighborBottom = chunk.GetNeighborBlock(x, y, z, Face::Bottom);
                if (isWater ? false : !IsOpaque(neighborBottom)) {
                    AddFace(targetMesh, chunk, x, y, z, Face::Bottom, block);
                }

                // North face (+Z) - For water, only render if NOT at chunk boundary
                BlockType neighborNorth = chunk.GetNeighborBlock(x, y, z, Face::North);
                bool northAtBoundary = (z == CHUNK_DEPTH - 1);
                if (isWater ? (!northAtBoundary && neighborNorth == BlockType::Air) : !IsOpaque(neighborNorth)) {
                    AddFace(targetMesh, chunk, x, y, z, Face::North, block);
                }

                // South face (-Z) - For water, only render if NOT at chunk boundary
                BlockType neighborSouth = chunk.GetNeighborBlock(x, y, z, Face::South);
                bool southAtBoundary = (z == 0);
                if (isWater ? (!southAtBoundary && neighborSouth == BlockType::Air) : !IsOpaque(neighborSouth)) {
                    AddFace(targetMesh, chunk, x, y, z, Face::South, block);
                }

                // East face (+X) - For water, only render if NOT at chunk boundary
                BlockType neighborEast = chunk.GetNeighborBlock(x, y, z, Face::East);
                bool eastAtBoundary = (x == CHUNK_WIDTH - 1);
                if (isWater ? (!eastAtBoundary && neighborEast == BlockType::Air) : !IsOpaque(neighborEast)) {
                    AddFace(targetMesh, chunk, x, y, z, Face::East, block);
                }

                // West face (-X) - For water, only render if NOT at chunk boundary
                BlockType neighborWest = chunk.GetNeighborBlock(x, y, z, Face::West);
                bool westAtBoundary = (x == 0);
                if (isWater ? (!westAtBoundary && neighborWest == BlockType::Air) : !IsOpaque(neighborWest)) {
                    AddFace(targetMesh, chunk, x, y, z, Face::West, block);
                }
            }
        }
    }

    return result;
}

void ChunkMeshBuilder::AddFace(ChunkMesh& mesh, 
                                const Chunk& chunk,
                                int x, int y, int z,
                                Face face, 
                                BlockType blockType) {
    glm::vec3 position(static_cast<float>(x), 
                       static_cast<float>(y), 
                       static_cast<float>(z));

    // Get the 4 vertices for this face
    glm::vec3 vertices[4];
    GetFaceVertices(face, position, vertices);

    // Get UV coordinates
    glm::vec2 uvs[4];
    GetFaceUVs(uvs);

    // Get normal for this face
    glm::vec3 normal = GetFaceNormal(face);

    // Calculate ambient occlusion for each vertex
    float ao[4];
    CalculateFaceAO(chunk, x, y, z, face, ao);

    // Get texture index based on block type and face
    float texIndex = static_cast<float>(GetTextureIndex(blockType, face));

    // Get tint color for this face (biome-based for grass/foliage)
    glm::vec3 tintColor = GetTintColor(ToBlockID(blockType), face);

    // Calculate sky light: blocks above the highest solid block in this column have sky access
    // For blocks at or below the heightmap, they're considered underground
    float skyLight = (y >= m_HeightMap[x][z]) ? 1.0f : 0.0f;

    // Current vertex index before adding new vertices
    unsigned int baseIndex = static_cast<unsigned int>(mesh.vertices.size());

    // Add 4 vertices for the face
    for (int i = 0; i < 4; ++i) {
        mesh.vertices.push_back({vertices[i], uvs[i], normal, ao[i], texIndex, tintColor, skyLight});
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

bool ChunkMeshBuilder::IsBlockOpaque(const Chunk& chunk, int x, int y, int z) {
    // Out of bounds is considered transparent (no occlusion)
    if (x < 0 || x >= CHUNK_WIDTH ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_DEPTH) {
        return false;
    }
    return IsOpaque(chunk.GetBlock(x, y, z));
}

float ChunkMeshBuilder::CalculateVertexAO(bool side1, bool side2, bool corner) {
    // Standard Minecraft-style ambient occlusion formula
    // If both sides are occluded, the corner is fully dark (prevents light bleeding)
    if (side1 && side2) {
        return 0.2f;  // Minimum AO to avoid pure black
    }
    // Count occluding neighbors: 0, 1, 2, or 3
    int occluders = (side1 ? 1 : 0) + (side2 ? 1 : 0) + (corner ? 1 : 0);
    // Map to AO values: 3->0.2, 2->0.5, 1->0.75, 0->1.0
    return 1.0f - (occluders * 0.25f) + 0.05f;
}

void ChunkMeshBuilder::CalculateFaceAO(const Chunk& chunk, int x, int y, int z, 
                                        Face face, float outAO[4]) {
    // For each vertex of the face, check 3 neighbors:
    // - 2 edge-adjacent blocks (sides)
    // - 1 corner-adjacent block
    // The vertex order matches GetFaceVertices()

    switch (face) {
        case Face::Top: // +Y face, vertices at corners of the top of the block
            // Vertex 0: (x, y+1, z) - corner at (-x, +y, -z)
            outAO[0] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y+1, z),   // side1: -X
                IsBlockOpaque(chunk, x, y+1, z-1),   // side2: -Z
                IsBlockOpaque(chunk, x-1, y+1, z-1)  // corner: -X,-Z
            );
            // Vertex 1: (x, y+1, z+1) - corner at (-x, +y, +z)
            outAO[1] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y+1, z),   // side1: -X
                IsBlockOpaque(chunk, x, y+1, z+1),   // side2: +Z
                IsBlockOpaque(chunk, x-1, y+1, z+1)  // corner: -X,+Z
            );
            // Vertex 2: (x+1, y+1, z+1) - corner at (+x, +y, +z)
            outAO[2] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y+1, z),   // side1: +X
                IsBlockOpaque(chunk, x, y+1, z+1),   // side2: +Z
                IsBlockOpaque(chunk, x+1, y+1, z+1)  // corner: +X,+Z
            );
            // Vertex 3: (x+1, y+1, z) - corner at (+x, +y, -z)
            outAO[3] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y+1, z),   // side1: +X
                IsBlockOpaque(chunk, x, y+1, z-1),   // side2: -Z
                IsBlockOpaque(chunk, x+1, y+1, z-1)  // corner: +X,-Z
            );
            break;

        case Face::Bottom: // -Y face
            // Vertex 0: (x, y, z+1)
            outAO[0] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y-1, z),
                IsBlockOpaque(chunk, x, y-1, z+1),
                IsBlockOpaque(chunk, x-1, y-1, z+1)
            );
            // Vertex 1: (x, y, z)
            outAO[1] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y-1, z),
                IsBlockOpaque(chunk, x, y-1, z-1),
                IsBlockOpaque(chunk, x-1, y-1, z-1)
            );
            // Vertex 2: (x+1, y, z)
            outAO[2] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y-1, z),
                IsBlockOpaque(chunk, x, y-1, z-1),
                IsBlockOpaque(chunk, x+1, y-1, z-1)
            );
            // Vertex 3: (x+1, y, z+1)
            outAO[3] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y-1, z),
                IsBlockOpaque(chunk, x, y-1, z+1),
                IsBlockOpaque(chunk, x+1, y-1, z+1)
            );
            break;

        case Face::North: // +Z face
            // Vertex 0: (x+1, y, z+1)
            outAO[0] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z+1),
                IsBlockOpaque(chunk, x, y-1, z+1),
                IsBlockOpaque(chunk, x+1, y-1, z+1)
            );
            // Vertex 1: (x+1, y+1, z+1)
            outAO[1] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z+1),
                IsBlockOpaque(chunk, x, y+1, z+1),
                IsBlockOpaque(chunk, x+1, y+1, z+1)
            );
            // Vertex 2: (x, y+1, z+1)
            outAO[2] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z+1),
                IsBlockOpaque(chunk, x, y+1, z+1),
                IsBlockOpaque(chunk, x-1, y+1, z+1)
            );
            // Vertex 3: (x, y, z+1)
            outAO[3] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z+1),
                IsBlockOpaque(chunk, x, y-1, z+1),
                IsBlockOpaque(chunk, x-1, y-1, z+1)
            );
            break;

        case Face::South: // -Z face
            // Vertex 0: (x, y, z)
            outAO[0] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z-1),
                IsBlockOpaque(chunk, x, y-1, z-1),
                IsBlockOpaque(chunk, x-1, y-1, z-1)
            );
            // Vertex 1: (x, y+1, z)
            outAO[1] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z-1),
                IsBlockOpaque(chunk, x, y+1, z-1),
                IsBlockOpaque(chunk, x-1, y+1, z-1)
            );
            // Vertex 2: (x+1, y+1, z)
            outAO[2] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z-1),
                IsBlockOpaque(chunk, x, y+1, z-1),
                IsBlockOpaque(chunk, x+1, y+1, z-1)
            );
            // Vertex 3: (x+1, y, z)
            outAO[3] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z-1),
                IsBlockOpaque(chunk, x, y-1, z-1),
                IsBlockOpaque(chunk, x+1, y-1, z-1)
            );
            break;

        case Face::East: // +X face
            // Vertex 0: (x+1, y, z)
            outAO[0] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z-1),
                IsBlockOpaque(chunk, x+1, y-1, z),
                IsBlockOpaque(chunk, x+1, y-1, z-1)
            );
            // Vertex 1: (x+1, y+1, z)
            outAO[1] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z-1),
                IsBlockOpaque(chunk, x+1, y+1, z),
                IsBlockOpaque(chunk, x+1, y+1, z-1)
            );
            // Vertex 2: (x+1, y+1, z+1)
            outAO[2] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z+1),
                IsBlockOpaque(chunk, x+1, y+1, z),
                IsBlockOpaque(chunk, x+1, y+1, z+1)
            );
            // Vertex 3: (x+1, y, z+1)
            outAO[3] = CalculateVertexAO(
                IsBlockOpaque(chunk, x+1, y, z+1),
                IsBlockOpaque(chunk, x+1, y-1, z),
                IsBlockOpaque(chunk, x+1, y-1, z+1)
            );
            break;

        case Face::West: // -X face
            // Vertex 0: (x, y, z+1)
            outAO[0] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z+1),
                IsBlockOpaque(chunk, x-1, y-1, z),
                IsBlockOpaque(chunk, x-1, y-1, z+1)
            );
            // Vertex 1: (x, y+1, z+1)
            outAO[1] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z+1),
                IsBlockOpaque(chunk, x-1, y+1, z),
                IsBlockOpaque(chunk, x-1, y+1, z+1)
            );
            // Vertex 2: (x, y+1, z)
            outAO[2] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z-1),
                IsBlockOpaque(chunk, x-1, y+1, z),
                IsBlockOpaque(chunk, x-1, y+1, z-1)
            );
            // Vertex 3: (x, y, z)
            outAO[3] = CalculateVertexAO(
                IsBlockOpaque(chunk, x-1, y, z-1),
                IsBlockOpaque(chunk, x-1, y-1, z),
                IsBlockOpaque(chunk, x-1, y-1, z-1)
            );
            break;
    }
}

void ChunkMeshBuilder::ComputeHeightMap(const Chunk& chunk) {
    // For each (x, z) column, find the highest non-transparent (solid) block
    // Blocks above this height have sky access
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int highestSolid = -1;  // -1 means no solid blocks in column
            // Scan from top down for efficiency (most columns have surface near top)
            for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                BlockType block = chunk.GetBlock(x, y, z);
                if (IsOpaque(block)) {
                    highestSolid = y;
                    break;
                }
            }
            m_HeightMap[x][z] = highestSolid + 1;  // +1 so blocks AT the surface have sky access
        }
    }
}

} // namespace Voxel
