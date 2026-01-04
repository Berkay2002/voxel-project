# Phase 4: Multi-Chunk World

## Goal

Create a `ChunkManager` to handle multiple chunks, enabling infinite terrain exploration with seamless chunk loading/unloading based on player position.

---

## New Files

### world/ChunkManager.h/.cpp

- Stores chunks in `std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>>`
- `ChunkCoord` = simple struct with `x, z` and hash function
- Key methods:
  - `Update(glm::vec3 playerPos)` — load/unload chunks within radius
  - `GetChunk(int cx, int cz)` — returns existing or generates new
  - `RenderAll(Shader& shader, Camera& camera)` — draw all loaded chunks

### world/ChunkMesh.h (optional refactor)

- Move VAO/VBO/IBO ownership from Engine to per-chunk
- Each chunk holds its own GPU buffers

---

## Modified Files

### core/Engine.h/.cpp

- Replace single `m_Chunk` with `ChunkManager`
- Update loop to call `ChunkManager::Update()` and `RenderAll()`
- Pass camera position to ChunkManager

### world/Chunk.h/.cpp

- Add mesh storage (VAO, VBO, IBO) or separate ChunkMesh
- Add `BuildMesh()` method that calls ChunkMeshBuilder + uploads to GPU

---

## Key Design Decisions

### Chunk Coordinate Hashing

```cpp
struct ChunkCoord {
    int x, z;
    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
};

struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& c) const {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 16);
    }
};
```

### Load Radius

- Start simple: `LOAD_RADIUS = 2` (5×5 grid = 25 chunks)
- `UNLOAD_RADIUS = LOAD_RADIUS + 1` (hysteresis to prevent thrashing)

### Cross-Chunk Face Culling

- When building mesh for chunk at boundary, query neighbor chunk
- If neighbor not loaded, treat as Air (render the face)
- When neighbor loads, mark adjacent chunk as dirty for rebuild

---

## Rendering Flow

```
Engine::Update()
├── ChunkManager::Update(cameraPos)
│   ├── Calculate current chunk coord from position
│   ├── For each chunk in load radius:
│   │   └── If not loaded: Generate + BuildMesh
│   └── For each loaded chunk outside unload radius:
│       └── Unload (GPU cleanup)
│
Engine::Render()
└── ChunkManager::RenderAll(shader, camera)
    └── For each loaded chunk:
        ├── Set model matrix (chunkX * 16, 0, chunkZ * 16)
        └── Draw chunk mesh
```

---

## Configuration

| Parameter     | Value | Notes                          |
| ------------- | ----- | ------------------------------ |
| LOAD_RADIUS   | 2     | 5×5 = 25 chunks loaded         |
| UNLOAD_RADIUS | 3     | Prevents load/unload thrashing |

---

## Validation

```bash
cd build && cmake .. && make -j$(nproc) && ./VoxelEngine
```

- Multiple chunks visible around player
- Terrain is seamless across chunk boundaries
- Flying far triggers new chunk loading
- No visible seams or missing faces at boundaries
