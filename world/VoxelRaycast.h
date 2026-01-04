#pragma once

#include "BlockRegistry.h"
#include "core/Ray.h"
#include <glm/glm.hpp>

namespace Voxel {

class ChunkManager;  // Forward declaration

/**
 * Result of a voxel raycast operation.
 * Contains information about what block was hit and where.
 */
struct RaycastResult {
    bool hit = false;              // Did the ray hit a solid block?
    glm::ivec3 blockPos{0};        // World position of the hit block
    glm::ivec3 previousPos{0};     // Position before entering hit block (for placing)
    Face hitFace = Face::Top;      // Which face of the block was hit
    BlockID blockType = BLOCK_AIR; // Type of block that was hit
    float distance = 0.0f;         // Distance from ray origin to hit point
};

/**
 * Cast a ray through the voxel world using the DDA (Digital Differential Analyzer) algorithm.
 * This efficiently steps through voxels along the ray path.
 * 
 * @param ray The ray to cast (origin + normalized direction)
 * @param world The chunk manager to query blocks from
 * @param maxDistance Maximum distance to check (default: 8 blocks, Minecraft reach distance)
 * @return RaycastResult with hit information
 */
RaycastResult Raycast(const Core::Ray& ray, ChunkManager& world, float maxDistance = 8.0f);

} // namespace Voxel
