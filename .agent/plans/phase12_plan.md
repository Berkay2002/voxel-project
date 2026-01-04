# Phase 12: Raycasting & Block Interaction

Implementation plan for adding DDA-based voxel raycasting with block breaking and placing.

## Goal

Enable player interaction with the voxel world:
- Cast rays from camera to detect which block is being looked at
- Left-click to break blocks (set to Air)
- Right-click to place blocks (adjacent to hit face)
- Visual feedback showing targeted block

## User Review Required

> [!IMPORTANT]
> **Block Placement Type**: Currently, the plan uses a hardcoded block type (Stone) for placing. Should we add:
> - A simple hotbar/inventory system in a future phase?
> - Or cycle through block types with scroll wheel for now?

---

## Proposed Changes

### Core Module — Ray Struct

#### [NEW] [Ray.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Ray.h)

Simple ray structure for raycasting operations:

```cpp
#pragma once
#include <glm/glm.hpp>

namespace Core {

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;  // Should be normalized
    
    Ray() : origin(0.0f), direction(0.0f, 0.0f, -1.0f) {}
    Ray(const glm::vec3& o, const glm::vec3& d) 
        : origin(o), direction(glm::normalize(d)) {}
    
    // Get point along ray at distance t
    [[nodiscard]] glm::vec3 GetPoint(float t) const {
        return origin + direction * t;
    }
};

} // namespace Core
```

---

### World Module — Voxel Raycasting

#### [NEW] [VoxelRaycast.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/VoxelRaycast.h)

Raycast result structure and function declarations:

```cpp
#pragma once
#include "BlockRegistry.h"
#include "core/Ray.h"
#include <glm/glm.hpp>

namespace Voxel {

class ChunkManager;  // Forward declaration

struct RaycastResult {
    bool hit = false;              // Did the ray hit a solid block?
    glm::ivec3 blockPos{0};        // World position of the hit block
    glm::ivec3 previousPos{0};     // Position before entering hit block (for placing)
    Face hitFace = Face::Top;      // Which face was hit
    BlockID blockType = BLOCK_AIR; // Type of block that was hit
    float distance = 0.0f;         // Distance from ray origin to hit point
};

/**
 * Cast a ray through the voxel world using DDA algorithm.
 * @param ray The ray to cast (origin + normalized direction)
 * @param world The chunk manager to query blocks from
 * @param maxDistance Maximum distance to check (default: 8 blocks, Minecraft standard)
 * @return RaycastResult with hit information
 */
RaycastResult Raycast(const Core::Ray& ray, ChunkManager& world, float maxDistance = 8.0f);

} // namespace Voxel
```

#### [NEW] [VoxelRaycast.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/VoxelRaycast.cpp)

DDA algorithm implementation (~80 lines):

```cpp
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
    
    // Direction signs (+1 or -1)
    glm::ivec3 step(
        (ray.direction.x >= 0) ? 1 : -1,
        (ray.direction.y >= 0) ? 1 : -1,
        (ray.direction.z >= 0) ? 1 : -1
    );
    
    // How far along ray to cross one voxel in each direction
    glm::vec3 tDelta(
        (ray.direction.x != 0) ? std::abs(1.0f / ray.direction.x) : 1e30f,
        (ray.direction.y != 0) ? std::abs(1.0f / ray.direction.y) : 1e30f,
        (ray.direction.z != 0) ? std::abs(1.0f / ray.direction.z) : 1e30f
    );
    
    // Distance to next voxel boundary in each direction
    glm::vec3 tMax;
    tMax.x = (ray.direction.x != 0) 
        ? ((step.x > 0 ? (current.x + 1 - ray.origin.x) : (ray.origin.x - current.x)) * tDelta.x)
        : 1e30f;
    tMax.y = (ray.direction.y != 0)
        ? ((step.y > 0 ? (current.y + 1 - ray.origin.y) : (ray.origin.y - current.y)) * tDelta.y)
        : 1e30f;
    tMax.z = (ray.direction.z != 0)
        ? ((step.z > 0 ? (current.z + 1 - ray.origin.z) : (ray.origin.z - current.z)) * tDelta.z)
        : 1e30f;
    
    glm::ivec3 previous = current;
    Face lastFace = Face::Top;
    float distance = 0.0f;
    
    // DDA loop
    while (distance < maxDistance) {
        // Check if current voxel is solid
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
        
        // Store previous position (for block placing)
        previous = current;
        
        // Step to nearest voxel boundary
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            distance = tMax.x;
            tMax.x += tDelta.x;
            current.x += step.x;
            lastFace = (step.x > 0) ? Face::West : Face::East;
        } else if (tMax.y < tMax.z) {
            distance = tMax.y;
            tMax.y += tDelta.y;
            current.y += step.y;
            lastFace = (step.y > 0) ? Face::Bottom : Face::Top;
        } else {
            distance = tMax.z;
            tMax.z += tDelta.z;
            current.z += step.z;
            lastFace = (step.z > 0) ? Face::South : Face::North;
        }
    }
    
    return result;  // No hit
}

} // namespace Voxel
```

---

### ChunkManager Modifications

#### [MODIFY] [ChunkManager.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.h)

Add block query and modification methods:

```diff
+ // Query a block at world coordinates
+ BlockID GetBlock(int worldX, int worldY, int worldZ) const;
+ 
+ // Set a block at world coordinates (triggers mesh rebuild)
+ void SetBlock(int worldX, int worldY, int worldZ, BlockID block);
+ 
+ private:
+   // Helper to mark chunk for mesh rebuild
+   void MarkChunkDirty(int chunkX, int chunkZ);
```

#### [MODIFY] [ChunkManager.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/ChunkManager.cpp)

Implement block access methods:

```cpp
BlockID ChunkManager::GetBlock(int worldX, int worldY, int worldZ) const {
    // Bounds check for Y
    if (worldY < 0 || worldY >= CHUNK_HEIGHT) {
        return BLOCK_AIR;
    }
    
    // Convert world coords to chunk coords
    int chunkX = (worldX >= 0) ? (worldX / CHUNK_SIZE) : ((worldX + 1) / CHUNK_SIZE - 1);
    int chunkZ = (worldZ >= 0) ? (worldZ / CHUNK_SIZE) : ((worldZ + 1) / CHUNK_SIZE - 1);
    
    // Local coords within chunk
    int localX = worldX - chunkX * CHUNK_SIZE;
    int localZ = worldZ - chunkZ * CHUNK_SIZE;
    
    // Find chunk
    auto* chunk = GetChunk(chunkX, chunkZ);
    if (!chunk) return BLOCK_AIR;
    
    return chunk->GetBlock(localX, worldY, localZ);
}

void ChunkManager::SetBlock(int worldX, int worldY, int worldZ, BlockID block) {
    // Bounds check
    if (worldY < 0 || worldY >= CHUNK_HEIGHT) return;
    
    // Convert to chunk coords
    int chunkX = (worldX >= 0) ? (worldX / CHUNK_SIZE) : ((worldX + 1) / CHUNK_SIZE - 1);
    int chunkZ = (worldZ >= 0) ? (worldZ / CHUNK_SIZE) : ((worldZ + 1) / CHUNK_SIZE - 1);
    
    int localX = worldX - chunkX * CHUNK_SIZE;
    int localZ = worldZ - chunkZ * CHUNK_SIZE;
    
    auto* chunk = GetChunk(chunkX, chunkZ);
    if (!chunk) return;
    
    chunk->SetBlock(localX, worldY, localZ, block);
    MarkChunkDirty(chunkX, chunkZ);
    
    // Check if we need to rebuild neighbor chunks (block on boundary)
    if (localX == 0) MarkChunkDirty(chunkX - 1, chunkZ);
    if (localX == CHUNK_SIZE - 1) MarkChunkDirty(chunkX + 1, chunkZ);
    if (localZ == 0) MarkChunkDirty(chunkX, chunkZ - 1);
    if (localZ == CHUNK_SIZE - 1) MarkChunkDirty(chunkX, chunkZ + 1);
}

void ChunkManager::MarkChunkDirty(int chunkX, int chunkZ) {
    auto* chunk = GetChunk(chunkX, chunkZ);
    if (chunk) {
        // Trigger async mesh rebuild
        LoadChunkAsync(chunkX, chunkZ, true);  // true = force rebuild
    }
}
```

---

### Chunk Modifications

#### [MODIFY] [Chunk.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/Chunk.h)

Add SetBlock method:

```diff
+ // Set a block (for player interaction)
+ void SetBlock(int x, int y, int z, BlockID block);
```

#### [MODIFY] [Chunk.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/Chunk.cpp)

```cpp
void Chunk::SetBlock(int x, int y, int z, BlockID block) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    m_Blocks[GetIndex(x, y, z)] = block;
}
```

---

### Camera Modification

#### [MODIFY] [Camera.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Camera.h)

Add method to get view ray:

```diff
+ #include "Ray.h"
+ 
+ // Get a ray from camera position in view direction
+ [[nodiscard]] Ray GetViewRay() const { return Ray(m_Position, m_Front); }
```

---

### Engine Integration

#### [MODIFY] [Engine.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.h)

Add raycast-related members:

```diff
+ #include "world/VoxelRaycast.h"
+ 
+ private:
+   // Block interaction
+   Voxel::RaycastResult m_TargetedBlock;  // Currently targeted block
+   BlockID m_SelectedBlockType = 3;        // Block type to place (Stone by default)
+   
+   // Input handling
+   void OnMouseButton(int button, int action);
+   void UpdateTargetedBlock();
```

#### [MODIFY] [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp)

Add mouse callback setup in constructor:

```cpp
// In Engine constructor, after window creation:
glfwSetWindowUserPointer(m_Window->GetHandle(), this);
glfwSetMouseButtonCallback(m_Window->GetHandle(), [](GLFWwindow* w, int button, int action, int mods) {
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
    engine->OnMouseButton(button, action);
});
```

Add interaction methods:

```cpp
void Engine::UpdateTargetedBlock() {
    if (m_Camera && m_ChunkManager) {
        Core::Ray ray = m_Camera->GetViewRay();
        m_TargetedBlock = Voxel::Raycast(ray, *m_ChunkManager, 8.0f);
    }
}

void Engine::OnMouseButton(int button, int action) {
    if (action != GLFW_PRESS || !m_CursorCaptured) return;
    
    if (!m_TargetedBlock.hit) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        // Break block
        m_ChunkManager->SetBlock(
            m_TargetedBlock.blockPos.x,
            m_TargetedBlock.blockPos.y,
            m_TargetedBlock.blockPos.z,
            Voxel::BLOCK_AIR
        );
        LOG_DEBUG("Broke block at " + 
            std::to_string(m_TargetedBlock.blockPos.x) + ", " +
            std::to_string(m_TargetedBlock.blockPos.y) + ", " +
            std::to_string(m_TargetedBlock.blockPos.z));
    } 
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        // Place block at previous position (empty space before hit)
        m_ChunkManager->SetBlock(
            m_TargetedBlock.previousPos.x,
            m_TargetedBlock.previousPos.y,
            m_TargetedBlock.previousPos.z,
            m_SelectedBlockType
        );
        LOG_DEBUG("Placed block at " +
            std::to_string(m_TargetedBlock.previousPos.x) + ", " +
            std::to_string(m_TargetedBlock.previousPos.y) + ", " +
            std::to_string(m_TargetedBlock.previousPos.z));
    }
}
```

Update the [Update()](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp#238-249) method:

```diff
void Engine::Update(float deltaTime) {
    glfwPollEvents();
    ProcessInput(deltaTime);
    
+   // Update targeted block for interaction
+   UpdateTargetedBlock();
    
    if (m_ChunkManager && m_Camera) {
        m_ChunkManager->Update(m_Camera->GetPosition());
        m_ChunkManager->ProcessPendingMeshes();
    }
}
```

---

### Build System

#### [MODIFY] [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt)

Add new source file:

```diff
set(WORLD_SOURCES
    world/Block.h
    world/BlockRegistry.h
    world/BlockRegistry.cpp
    world/Chunk.h
    world/Chunk.cpp
    world/ChunkManager.h
    world/ChunkManager.cpp
    world/ChunkMeshBuilder.h
    world/ChunkMeshBuilder.cpp
    world/ChunkTask.h
+   world/VoxelRaycast.h
+   world/VoxelRaycast.cpp
    # ... rest of files
)
```

---

## Optional Enhancements (Phase 12B)

These can be added after core functionality works:

### Crosshair Overlay
- Simple 2D shader rendering a + at screen center
- Requires orthographic projection for UI

### Block Highlight
- Render wireframe cube at `m_TargetedBlock.blockPos`
- Slightly larger than 1x1x1 to avoid z-fighting
- Use `GL_LINES` with a simple shader

### Block Type Selection
- Scroll wheel to cycle through available blocks
- Simple HUD showing current block type

---

## Verification Plan

### Automated Tests
```bash
# Build the project
cd build && cmake .. && make -j$(nproc)

# Run and verify no crashes
./VoxelEngine
```

### Manual Verification
1. ✅ Press M to capture mouse
2. ✅ Look at a block — verify raycast hits (check debug logs)
3. ✅ Left-click — block should disappear, mesh updates
4. ✅ Right-click — stone block should appear
5. ✅ Break blocks on chunk boundaries — verify neighbor chunks update
6. ✅ Try breaking bedrock at Y=0 — should work
7. ✅ Try placing in the air (no target) — should do nothing

---

## File Summary

| Action | File | Description |
|--------|------|-------------|
| **NEW** | `core/Ray.h` | Ray struct |
| **NEW** | `world/VoxelRaycast.h` | Raycast result + function declaration |
| **NEW** | `world/VoxelRaycast.cpp` | DDA algorithm implementation |
| **MODIFY** | [core/Camera.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Camera.h) | Add `GetViewRay()` method |
| **MODIFY** | [core/Engine.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.h) | Add raycast members and methods |
| **MODIFY** | [core/Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp) | Mouse callbacks, block interaction |
| **MODIFY** | `world/Chunk.h/.cpp` | Add `SetBlock()` method |
| **MODIFY** | `world/ChunkManager.h/.cpp` | Add [GetBlock()](file:///home/berkay-orhan/Developer/playground/voxel-project/world/Block.h#76-80), `SetBlock()`, `MarkChunkDirty()` |
| **MODIFY** | [CMakeLists.txt](file:///home/berkay-orhan/Developer/playground/voxel-project/CMakeLists.txt) | Add new source files |
