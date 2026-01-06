#pragma once

#include <vector>
#include <atomic>
#include <glm/glm.hpp>

namespace Voxel {

// Vertex structure for chunk mesh  
// Defined here to avoid circular dependency with ChunkMeshBuilder.h
struct ChunkVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    float ao;           // Ambient occlusion: 0.0 (fully occluded) to 1.0 (fully lit)
    float texIndex;     // Texture array layer index (0=grass_top, 1=dirt, etc.)
    glm::vec3 tintColor; // Tint color multiplied with texture (for grass/foliage biome tinting)
    float skyLight;     // Sky light exposure: 0.0 (underground) to 1.0 (open sky)
};

/**
 * Mesh data generated on a worker thread.
 * Contains raw vertex/index data that will be uploaded to GPU on main thread.
 * Separated into opaque (solid blocks) and water (transparent) meshes for proper rendering.
 * 
 * Performance: Uses structured ChunkVertex directly to avoid serialization overhead.
 */
struct ChunkMeshData {
    // Opaque mesh (solid blocks: dirt, grass, stone, etc.)
    std::vector<ChunkVertex> opaqueVertices;
    std::vector<unsigned int> opaqueIndices;
    
    // Water mesh (transparent blocks, rendered with alpha blending)
    std::vector<ChunkVertex> waterVertices;
    std::vector<unsigned int> waterIndices;
    
    int chunkX = 0;
    int chunkZ = 0;
    bool valid = true;  // Set to false if generation failed
};

/**
 * Chunk lifecycle state (thread-safe via atomic).
 * Tracks the loading/generation state of each chunk.
 */
enum class ChunkState {
    Unloaded,       // Not loaded, no block data
    Generating,     // Background thread generating terrain blocks
    Meshing,        // Background thread building mesh from blocks
    MeshPending,    // Mesh data ready in queue, needs GPU upload
    Ready,          // Uploaded to GPU, renderable
    Error           // Something went wrong during generation
};

/**
 * Convert ChunkState to string for debug logging.
 */
inline const char* ChunkStateToString(ChunkState state) {
    switch (state) {
        case ChunkState::Unloaded:      return "Unloaded";
        case ChunkState::Generating:    return "Generating";
        case ChunkState::Meshing:       return "Meshing";
        case ChunkState::MeshPending:   return "MeshPending";
        case ChunkState::Ready:         return "Ready";
        case ChunkState::Error:         return "Error";
        default:                        return "Unknown";
    }
}

} // namespace Voxel
