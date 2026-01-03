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

## Phase 3B: Terrain Generation (Current)

- [ ] **FastNoiseLite Integration**
  - [ ] Add to CMakeLists.txt via FetchContent
  - [ ] Verify include path works
- [ ] **TerrainGenerator Class**
  - [ ] Create `world/TerrainGenerator.h/.cpp`
  - [ ] Configure Perlin noise (seed, frequency)
  - [ ] `Generate(Chunk&)` method
- [ ] **Block Layering Logic**
  - [ ] Height calculation from noise
  - [ ] Stone / Dirt / Grass layers
- [ ] **Integration & Validation**
  - [ ] Replace Engine.cpp manual terrain with TerrainGenerator
  - [ ] Verify rolling hills render correctly

## Phase 4: Optimization & World (Backlog)

- [ ] Multithreading for chunk generation
- [ ] ChunkManager for multi-chunk world
- [ ] Infinite terrain loading
