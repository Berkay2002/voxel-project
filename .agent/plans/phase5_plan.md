# Phase 5: Performance Optimization

## Overview

Phase 5 focuses on two major performance improvements:

1.  **Frustum Culling** - Skip rendering chunks outside the camera's view
2.  **Multithreaded Chunk Generation** - Generate terrain and build meshes on background threads

These optimizations will significantly improve frame rates and eliminate hitching when loading chunks.

---

## Threading Strategy

Based on voxel engine best practices:

> [!NOTE] > **BS::thread_pool** for chunk work (generation, meshing, lighting) - handles bursty batches of short/medium tasks with proper work queue and thread reuse.

> [!NOTE] > **Mesh Upload Strategy**: GPU buffer uploads (OpenGL calls) MUST happen on the main thread. Worker threads generate vertex data into CPU buffers, then main thread uploads to GPU when ready.

### Why BS::thread_pool?

- Header-only, integrates via FetchContent
- Fixed worker pool (avoids per-task thread creation overhead)
- Supports `submit_task` (with future) and `detach_task` (fire-and-forget)
- `submit_loop` / `detach_loop` helpers map well to per-chunk batch work
- Properly handles bursty workloads typical in voxel engines

---

## Proposed Changes

### Part A: Frustum Culling (Simpler, Immediate Wins)

#### [NEW] [Frustum.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Frustum.h)

New class to extract and store the 6 frustum planes from view-projection matrix:

```cpp
class Frustum {
public:
    void ExtractPlanes(const glm::mat4& viewProj);
    bool IsAABBVisible(const glm::vec3& min, const glm::vec3& max) const;

private:
    struct Plane {
        glm::vec3 normal;
        float distance;
    };
    std::array<Plane, 6> m_Planes; // Left, Right, Bottom, Top, Near, Far
};
```

#### [MODIFY] [Camera.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Camera.h)

Add frustum getter:

```diff
+ #include "Frustum.h"

  class Camera {
  public:
+   const Frustum& GetFrustum() const { return m_Frustum; }
+   void UpdateFrustum(float aspectRatio);

  private:
+   Frustum m_Frustum;
  };
```

#### [MODIFY] [ChunkManager.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.cpp)

Add frustum check before rendering each chunk:

```diff
  void ChunkManager::RenderAll(...) {
+     camera.UpdateFrustum(aspectRatio);
+     const Frustum& frustum = camera.GetFrustum();
+     int culledCount = 0;

      for (const auto& [coord, chunk] : m_Chunks) {
+         // Calculate chunk AABB
+         glm::vec3 minBounds(worldX, 0, worldZ);
+         glm::vec3 maxBounds(worldX + CHUNK_WIDTH, CHUNK_HEIGHT, worldZ + CHUNK_DEPTH);
+
+         if (!frustum.IsAABBVisible(minBounds, maxBounds)) {
+             culledCount++;
+             continue;  // Skip - chunk is outside view
+         }

          // Existing render code...
      }
  }
```

---

### Part B: Multithreaded Chunk Generation

#### [MODIFY] [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt)

Add BS::thread_pool via FetchContent:

```cmake
# BS::thread_pool - Header-only thread pool library
FetchContent_Declare(
    thread_pool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(thread_pool)

# Add include path for BS_thread_pool.hpp
target_include_directories(${PROJECT_NAME} PRIVATE ${thread_pool_SOURCE_DIR}/include)
```

#### [NEW] [ChunkTask.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkTask.h)

Struct for passing generated mesh data back to main thread:

```cpp
#pragma once

#include <vector>
#include <atomic>

namespace Voxel {

// Mesh data generated on worker thread, uploaded on main thread
struct ChunkMeshData {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int chunkX = 0;
    int chunkZ = 0;
};

// Chunk lifecycle state (thread-safe via atomic)
enum class ChunkState {
    Unloaded,       // Not loaded
    Generating,     // Background thread generating terrain
    Meshing,        // Background thread building mesh
    MeshPending,    // Mesh data ready, needs GPU upload
    Ready           // Uploaded to GPU, renderable
};

} // namespace Voxel
```

#### [MODIFY] [Chunk.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/Chunk.h)

Add thread-safe state management:

```diff
+ #include "ChunkTask.h"
+ #include <atomic>

  class Chunk {
  public:
+   // Thread-safe state management
+   ChunkState GetState() const { return m_State.load(); }
+   void SetState(ChunkState state) { m_State.store(state); }
+
+   // Generate mesh data (thread-safe, no OpenGL calls)
+   ChunkMeshData GenerateMeshData() const;
+
+   // Upload mesh from pre-generated data (main thread only)
+   void UploadMeshFromData(const ChunkMeshData& data);

  private:
+   std::atomic<ChunkState> m_State{ChunkState::Unloaded};
  };
```

#### [MODIFY] [ChunkManager.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.h)

Add BS::thread_pool and pending mesh queue:

```diff
+ #include "ChunkTask.h"
+ #include <BS_thread_pool.hpp>
+ #include <queue>
+ #include <mutex>

  class ChunkManager {
  public:
+   // Process completed mesh tasks (call from main thread each frame)
+   void ProcessPendingMeshes();

  private:
+   // Thread pool for background chunk work
+   BS::thread_pool m_ThreadPool;
+
+   // Thread-safe queue for completed mesh data
+   std::queue<ChunkMeshData> m_PendingMeshes;
+   std::mutex m_PendingMeshMutex;
+
+   // Request async chunk loading
+   void LoadChunkAsync(int chunkX, int chunkZ);
  };
```

#### [MODIFY] [ChunkManager.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.cpp)

Implement async loading workflow using BS::thread_pool:

```cpp
void ChunkManager::LoadChunkAsync(int cx, int cz) {
    // Create chunk immediately (for state tracking)
    auto chunk = std::make_unique<Chunk>();
    chunk->SetPosition(cx, cz);
    chunk->SetState(ChunkState::Generating);

    Chunk* rawPtr = chunk.get();
    m_Chunks[{cx, cz}] = std::move(chunk);

    // Submit task to thread pool (fire-and-forget)
    m_ThreadPool.detach_task([this, rawPtr, cx, cz]() {
        // Generate terrain (thread-safe)
        m_TerrainGenerator.Generate(*rawPtr);
        rawPtr->SetState(ChunkState::Meshing);

        // Build mesh data (no OpenGL calls)
        ChunkMeshData meshData = rawPtr->GenerateMeshData();
        meshData.chunkX = cx;
        meshData.chunkZ = cz;

        // Queue for main thread upload
        {
            std::lock_guard lock(m_PendingMeshMutex);
            m_PendingMeshes.push(std::move(meshData));
        }

        rawPtr->SetState(ChunkState::MeshPending);
    });
}

void ChunkManager::ProcessPendingMeshes() {
    std::lock_guard lock(m_PendingMeshMutex);

    // Rate-limit uploads to avoid frame hitches
    constexpr int MAX_UPLOADS_PER_FRAME = 2;
    int processed = 0;

    while (!m_PendingMeshes.empty() && processed < MAX_UPLOADS_PER_FRAME) {
        ChunkMeshData& data = m_PendingMeshes.front();

        Chunk* chunk = GetChunk(data.chunkX, data.chunkZ);
        if (chunk && chunk->GetState() == ChunkState::MeshPending) {
            chunk->UploadMeshFromData(data);
            chunk->SetState(ChunkState::Ready);
        }

        m_PendingMeshes.pop();
        processed++;
    }
}
```

#### [MODIFY] [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp)

Call `ProcessPendingMeshes()` each frame:

```diff
  void Engine::Update(float deltaTime) {
      m_ChunkManager->Update(m_Camera->GetPosition());
+     m_ChunkManager->ProcessPendingMeshes();
  }
```

---

## File Summary

| File                        | Action | Description                                                             |
| --------------------------- | ------ | ----------------------------------------------------------------------- |
| `CMakeLists.txt`            | MODIFY | Add BS::thread_pool via FetchContent                                    |
| `core/Frustum.h`            | NEW    | Frustum plane extraction and AABB testing                               |
| `world/ChunkTask.h`         | NEW    | ChunkMeshData and ChunkState definitions                                |
| `core/Camera.h/.cpp`        | MODIFY | Add frustum caching                                                     |
| `world/Chunk.h/.cpp`        | MODIFY | Add thread-safe state, separate mesh generation from upload             |
| `world/ChunkManager.h/.cpp` | MODIFY | Async loading with BS::thread_pool, pending mesh queue, frustum culling |
| `core/Engine.cpp`           | MODIFY | Call ProcessPendingMeshes()                                             |

---

## Verification Plan

### Automated Tests

1.  **Build Test**:

    ```bash
    cd build && cmake .. && make -j$(nproc)
    ```

2.  **Run and Observe**:
    - Application should start without crashes
    - Chunks should appear progressively (async loading)
    - No visual glitches at chunk boundaries

### Manual Verification

1.  **Frustum Culling**:

    - Add debug counter for culled vs rendered chunks
    - Turn camera away from terrain → chunk render count should drop

2.  **Multithreading**:

    - Move rapidly through world → no frame hitches
    - Chunks appear smoothly in the background
    - Verify no race conditions (run for extended period)

3.  **Performance Metrics**:
    - Compare FPS before/after with `LOG_INFO` timings
    - Expected: 2-3x improvement in render time from culling
    - Expected: Eliminated load hitches from async generation

---

## Implementation Order

1.  **Part A: Frustum Culling** (simpler, fewer changes)

    - Create `Frustum.h`
    - Modify `Camera` to cache frustum
    - Add AABB test in `RenderAll()`

2.  **Part B: Multithreading** (more complex)
    - Add BS::thread_pool to CMakeLists.txt
    - Create `ChunkTask.h`
    - Refactor `Chunk` for thread safety
    - Modify `ChunkManager` for async workflow with `detach_task`
    - Update `Engine` loop
