#include "VoxelRaycast.h"
#include "ChunkManager.h"
#include <cmath>

namespace Voxel {

RaycastResult Raycast(const Core::Ray& ray, ChunkManager& world, float maxDistance) {
    RaycastResult result;
    
    // Current voxel position (floor to get integer coords)
    glm::ivec3 current(
        static_cast<int>(std::floor(ray.origin.x)),
        static_cast<int>(std::floor(ray.origin.y)),
        static_cast<int>(std::floor(ray.origin.z))
    );
    
    // Direction signs: +1 if positive, -1 if negative
    glm::ivec3 step(
        (ray.direction.x >= 0) ? 1 : -1,
        (ray.direction.y >= 0) ? 1 : -1,
        (ray.direction.z >= 0) ? 1 : -1
    );
    
    // tDelta: How far along the ray we must travel to cross one voxel in each direction
    // Uses a large value (1e30) for axes with zero direction to effectively ignore them
    glm::vec3 tDelta(
        (ray.direction.x != 0) ? std::abs(1.0f / ray.direction.x) : 1e30f,
        (ray.direction.y != 0) ? std::abs(1.0f / ray.direction.y) : 1e30f,
        (ray.direction.z != 0) ? std::abs(1.0f / ray.direction.z) : 1e30f
    );
    
    // tMax: Distance along ray to the next voxel boundary in each direction
    glm::vec3 tMax;
    
    if (ray.direction.x != 0) {
        float boundary = (step.x > 0) ? (current.x + 1.0f) : static_cast<float>(current.x);
        tMax.x = (boundary - ray.origin.x) / ray.direction.x;
    } else {
        tMax.x = 1e30f;
    }
    
    if (ray.direction.y != 0) {
        float boundary = (step.y > 0) ? (current.y + 1.0f) : static_cast<float>(current.y);
        tMax.y = (boundary - ray.origin.y) / ray.direction.y;
    } else {
        tMax.y = 1e30f;
    }
    
    if (ray.direction.z != 0) {
        float boundary = (step.z > 0) ? (current.z + 1.0f) : static_cast<float>(current.z);
        tMax.z = (boundary - ray.origin.z) / ray.direction.z;
    } else {
        tMax.z = 1e30f;
    }
    
    glm::ivec3 previous = current;
    Face lastFace = Face::Top;
    float distance = 0.0f;
    
    // DDA loop: step through voxels until we hit something or exceed max distance
    while (distance < maxDistance) {
        // Check if current voxel is solid (not air and has geometry)
        BlockID block = world.GetBlock(current.x, current.y, current.z);
        
        if (block != BLOCK_AIR && BlockRegistry::Instance().IsSolid(block)) {
            result.hit = true;
            result.blockPos = current;
            result.previousPos = previous;
            result.hitFace = lastFace;
            result.blockType = block;
            result.distance = distance;
            return result;
        }
        
        // Store previous position (used for block placing)
        previous = current;
        
        // Step to the nearest voxel boundary
        // The axis with the smallest tMax value is the next boundary we'll cross
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            // Step along X axis
            distance = tMax.x;
            tMax.x += tDelta.x;
            current.x += step.x;
            // Face we entered from (opposite of step direction)
            lastFace = (step.x > 0) ? Face::West : Face::East;
        } else if (tMax.y < tMax.z) {
            // Step along Y axis
            distance = tMax.y;
            tMax.y += tDelta.y;
            current.y += step.y;
            lastFace = (step.y > 0) ? Face::Bottom : Face::Top;
        } else {
            // Step along Z axis
            distance = tMax.z;
            tMax.z += tDelta.z;
            current.z += step.z;
            lastFace = (step.z > 0) ? Face::South : Face::North;
        }
    }
    
    // No hit within max distance
    return result;
}

} // namespace Voxel
