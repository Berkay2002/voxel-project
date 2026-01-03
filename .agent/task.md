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

## Phase 6: Lighting ✅

- [x] **Shader System Update**
  - [x] Create `assets/shaders/lit.vert` with normal passing
  - [x] Create `assets/shaders/lit.frag` with diffuse + ambient lighting
  - [x] Add `u_LightDir` and `u_AmbientStrength` uniforms
- [x] **Engine Integration**
  - [x] Load new lit shader in `Engine.cpp`
  - [x] Set light direction uniform (sun angle)
  - [x] Set ambient strength uniform
- [x] **Per-Vertex Ambient Occlusion**
  - [x] Add `float ao` field to `ChunkVertex` struct
  - [x] Calculate AO per vertex in `ChunkMeshBuilder`
  - [x] Pass AO to fragment shader and apply to lighting
- [x] **Validation**
  - [x] Build succeeds
  - [x] Blocks show directional shading (lighter tops, darker sides)
  - [x] AO creates subtle shadows in corners/edges

## Phase 7: Water System ✅

### Block System

- [x] Add `Water` to `BlockType` enum in `Block.h`
- [x] Update `IsOpaque()` to return `false` for Water
- [x] Update `IsSolid()` to return `true` for Water (has geometry, not collidable logic)
- [x] Add `IsTransparent()` helper function
- [x] Update `GetTextureIndex()` to return index 4 for Water

### Terrain Generation

- [x] Define `seaLevel` config field (set to 50 for visible water)
- [x] Update `TerrainGenerator::Generate()` to fill Water from terrain height to sea level

### Mesh Building

- [x] Add `ChunkMeshResult` struct with opaque + water meshes
- [x] Update `ChunkMeshBuilder::BuildMesh()` to return `ChunkMeshResult`
- [x] Separate opaque and water face generation logic
- [x] Fix chunk boundary artifacts by skipping water side faces at edges

### Chunk System

- [x] Extend `ChunkMeshData` with separate opaque/water vertex/index arrays
- [x] Add water GPU resources (m_WaterVAO, m_WaterVBO, m_WaterIBO)
- [x] Update `GenerateMeshData()` to produce both meshes
- [x] Update `UploadMeshFromData()` to upload both meshes
- [x] Add `RenderWater()` method
- [x] Update `CleanupMesh()` for water resources

### Shaders

- [x] Create `assets/shaders/water.vert`
- [x] Create `assets/shaders/water.frag` with blue tint and alpha

### Chunk Manager

- [x] Add `RenderWater()` method with frustum culling

### Engine Integration

- [x] Add `m_WaterShader` member
- [x] Load water shader in `SetupWorld()`
- [x] Update `Render()` with two-pass rendering (opaque → blended water)
- [x] Enable/disable GL_BLEND and depth mask correctly

### Assets & Validation

- [x] Verify build succeeds
- [x] Verify water renders at sea level
- [x] Verify transparency and blue tint
- [x] Verify no visual regression for opaque blocks
- [x] Fix chunk boundary dark band artifacts

## Phase 8A: Cave Generation (Spaghetti Caves) ✅

### Modular Cave System

- [x] Create `world/ICaveCarver.h` interface (abstract base for all cave types)
- [x] Create `world/SpaghettiCaveCarver.h/.cpp` (Minecraft-style winding tunnels)

### TerrainGenerator Updates

- [x] Add `enableCaves` config option
- [x] Add `AddCaveCarver()` / `ClearCaveCarvers()` methods
- [x] Implement two-pass generation (terrain first, then cave carving)
- [x] Water flooding for caves below sea level

### Integration

- [x] Update `CMakeLists.txt` with new cave files
- [x] Add cave carver to `ChunkManager` on initialization

### Validation

- [x] Build succeeds
- [x] Caves appear underground (runtime test)
- [x] Winding tunnels are organic and connected
- [x] Natural hillside cave entrances (surfaceProtection=0, dramatic terrain)
- [x] Underwater caves flood with water

### Centralized Configuration

- [x] Create `world/WorldConfig.h` with all tunable parameters
- [x] Update `TerrainConfig` defaults to use WorldConfig constants
- [x] Update `SpaghettiCaveConfig` defaults to use WorldConfig constants
- [x] Update `ChunkManager` to use WorldConfig constants

## Phase 8B: Cheese & Noodle Caves (Backlog)

- [ ] **Cheese Caves**: Large open caverns with pillars
  - [ ] `CheeseCaveCarver.h/.cpp`
  - [ ] Lower frequency noise, higher threshold
- [ ] **Noodle Caves**: Thin connecting tunnels
  - [ ] `NoodleCaveCarver.h/.cpp`
  - [ ] High frequency noise, low threshold

## Phase 8C: More World Features (Backlog)

- [ ] **Cave Generation Enhancements**
  - [ ] Ore vein distribution at specific depths
- [ ] **More Block Types**
  - [ ] Sand, Gravel, Cobblestone
  - [ ] Texture atlas for multiple block textures
- [ ] **Advanced Lighting** (Optional)
  - [ ] Sunlight propagation
  - [ ] Block light sources (torches)

## Phase 9: Biome System & Rivers ✅

- [x] **Biome System**
  - [x] Create `world/Biome.h` with BiomeType enum (Plains, Mountains)
  - [x] Define biome parameters (baseHeight, amplitude, layer composition)
  - [x] Integrate biome sampling into `TerrainGenerator` via low-frequency noise
- [x] **River System**
  - [x] Add cellular noise layer for winding river paths
  - [x] Carve river channels where noise exceeds threshold
  - [x] Fill carved areas with water at appropriate depth
  - [x] Disable rivers in mountain biomes (too steep)
- [x] **Update WorldConfig.h**
  - [x] Add biome noise parameters (BIOME_FREQUENCY)
  - [x] Add river generation parameters (RIVER_FREQUENCY, THRESHOLD, DEPTH)
- [x] **Validation**
  - [x] Build succeeds
  - [ ] Plains and Mountains biomes visible with distinct terrain (runtime)
  - [ ] Rivers flow through Plains as distinct water features (runtime)
