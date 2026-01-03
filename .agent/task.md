# Project Tasks

## Phase 1: Initialization & Core Engine ✅

Focus: Getting a window open, OpenGL context running, and basic engine loop.

- [x] **Project Layout & Build System**
  - [x] Create `.gitignore`
  - [x] Set up `CMakeLists.txt`
  - [x] Configure dependencies (GLFW, GLAD, GLM via FetchContent)
- [x] **Core Architecture**
  - [x] Implement `Core/Window` class (GLFW handling)
  - [x] Implement `Core/Engine` class (Main loop)
  - [x] Implement `Core/Logger` (Basic console logging)
- [x] **Graphics Verification**
  - [x] Initialize OpenGL 4.6 (GLAD)
  - [x] Clear screen with teal color
  - [ ] Render a test triangle (sanity check)

## Phase 2: Rendering Foundations ✅

- [x] **Shader System**
  - [x] Implement `core/Shader.h/.cpp`
  - [x] Create `assets/shaders/basic.vert/frag`
  - [x] Create `assets/shaders/textured.vert/frag`
- [x] **Buffer Abstractions**
  - [x] Implement `core/VertexBuffer.h/.cpp`
  - [x] Implement `core/IndexBuffer.h/.cpp`
  - [x] Implement `core/VertexArray.h/.cpp`
- [x] **Colored Triangle** - Validated shaders + buffers
- [x] **Texture System**
  - [x] Add stb_image dependency
  - [x] Implement `core/Texture.h/.cpp`
- [x] **Textured Quad** - Validated textures + UVs
- [x] **Camera System**
  - [x] Implement `core/Camera.h/.cpp`
  - [x] WASD + mouse look controls
- [x] **Block Preview Cube** - Full 3D validation

## Phase 3A: Chunk Data & Single Chunk Rendering ✅

- [x] **Block System**
  - [x] Create `world/Block.h` with BlockType enum (Air, Dirt, Grass, Stone)
  - [x] Add `IsOpaque()` and UV helpers
- [x] **Chunk Data Structure**
  - [x] Create `world/Chunk.h/.cpp` (16×16×256)
  - [x] 1D array storage with Get/Set accessors
- [x] **Mesh Generation with Face Culling**
  - [x] Create `world/ChunkMeshBuilder.h/.cpp`
  - [x] Implement 6-face iteration with neighbor checks
  - [x] Generate vertex data (pos, uv, normal)
- [x] **Integration**
  - [x] Update CMakeLists.txt for world/ sources
  - [x] Update Engine.cpp to create and render test chunk
- [x] **Validation**
  - [x] Single chunk renders correctly
  - [x] Internal faces are culled
  - [x] Block textures display correctly

## Phase 3B: Terrain Generation ✅

- [x] **FastNoiseLite Integration**
  - [x] Add to CMakeLists.txt via FetchContent
  - [x] Verify include path works
- [x] **TerrainGenerator Class**
  - [x] Create `world/TerrainGenerator.h/.cpp`
  - [x] Configure Perlin noise (seed, frequency)
  - [x] `Generate(Chunk&)` method
- [x] **Block Layering Logic**
  - [x] Height calculation from noise
  - [x] Stone / Dirt / Grass layers
- [x] **Integration & Validation**
  - [x] Replace Engine.cpp manual terrain with TerrainGenerator
  - [x] Verify rolling hills render correctly

## Phase 4: Multi-Chunk World ✅

- [x] **ChunkManager Class**
  - [x] Create `world/ChunkManager.h/.cpp`
  - [x] Hash map storage: `(chunkX, chunkZ)` → `Chunk*`
  - [x] `GetChunk(cx, cz)` with lazy generation
  - [x] `Update(playerPos)` for load/unload radius
- [x] **Chunk Mesh Storage**
  - [x] Move VAO/VBO/IBO into Chunk or ChunkMesh struct
  - [x] Per-chunk GPU buffers
- [x] **Rendering Multiple Chunks**
  - [x] ChunkManager::RenderAll() iterates visible chunks
  - [x] Model matrix offset per chunk
- [x] **Cross-Chunk Face Culling**
  - [x] Query neighbor chunks at chunk boundaries
  - [x] Rebuild mesh when neighbor loads
- [x] **Validation**
  - [x] 3x3 grid of chunks loads correctly
  - [x] Seamless terrain across chunk boundaries
  - [x] Walking triggers load/unload

## Phase 5: Performance Optimization (In Progress)

### Part A: Frustum Culling

- [x] **Frustum System**
  - [x] Create `core/Frustum.h` with plane extraction from view-projection matrix
  - [x] Implement `IsAABBVisible()` for chunk bounding box tests
- [x] **Camera Integration**
  - [x] Add `Frustum` member to `Camera` class
  - [x] Add `UpdateFrustum(aspectRatio)` method
- [x] **ChunkManager Integration**
  - [x] Add frustum test before rendering each chunk
  - [x] Add debug logging for culled chunk count
- [x] **Validation**
  - [x] Build succeeds
  - [ ] Verify chunks behind camera are not rendered (runtime test)

### Part B: Multithreaded Chunk Generation

- [x] **Thread Pool**
  - [x] Add BS::thread_pool to CMakeLists.txt via FetchContent
  - [x] Verify header include path works
- [x] **Chunk Task System**
  - [x] Create `world/ChunkTask.h` with `ChunkMeshData` struct
  - [x] Define `ChunkState` enum (Unloaded, Generating, MeshPending, Ready)
- [x] **Chunk Refactoring**
  - [x] Add `std::atomic<ChunkState>` to `Chunk`
  - [x] Create `GenerateMeshData()` (thread-safe, no OpenGL)
  - [x] Create `UploadMeshFromData()` (main thread only)
- [x] **ChunkManager Async Loading**
  - [x] Add `m_ThreadPool` and `m_PendingMeshes` queue
  - [x] Implement `LoadChunkAsync()` to enqueue background tasks
  - [x] Implement `ProcessPendingMeshes()` for main thread GPU upload
  - [x] Rate-limit uploads (max 2 per frame) to avoid hitches
- [x] **Engine Integration**
  - [x] Call `ProcessPendingMeshes()` in `Engine::Update()`
- [x] **Validation**
  - [x] Build succeeds
  - [ ] Verify chunks appear progressively (runtime test)
  - [ ] Verify no frame hitches when moving rapidly (runtime test)

## Phase 6: World Features (Backlog)

- [ ] **Water System**
  - [ ] Water BlockType at sea level
  - [ ] Transparent rendering (alpha blending)
  - [ ] Animated water UVs (optional)
- [ ] **Cave Generation**
  - [ ] 3D Perlin noise for cave carving
  - [ ] Ore vein distribution
- [ ] **More Block Types**
  - [ ] Sand, Gravel, Cobblestone
  - [ ] Texture atlas for multiple block textures
- [ ] **Lighting**
  - [ ] Basic ambient + directional sun
  - [ ] Per-vertex ambient occlusion
