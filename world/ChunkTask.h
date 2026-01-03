#pragma once

#include <vector>
#include <atomic>

namespace Voxel {

/**
 * Mesh data generated on a worker thread.
 * Contains raw vertex/index data that will be uploaded to GPU on main thread.
 */
struct ChunkMeshData {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
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
