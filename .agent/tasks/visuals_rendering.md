# Phase 10–16A: Visuals, Rendering, Interaction

## Phase 10A: Per-Vertex Block Colors ✅

- [x] **ChunkVertex Color Attribute**
  - [x] Add `glm::vec3 color` to `ChunkVertex` struct
  - [x] Update all vertex attribute setups in `Chunk.cpp` (sync + async paths)
- [x] **Block Color Helper**
  - [x] Add `GetBlockColor(BlockType, Face)` to `Block.h`
  - [x] Distinct colors: Grass=green, Dirt=brown, Stone=gray, Water=blue
- [x] **Mesh Builder Integration**
  - [x] Set vertex color in `ChunkMeshBuilder::AddFace()`
- [x] **Shader Updates**
  - [x] Update `lit.vert/frag` to use vertex color at location 4
  - [x] Update `water.vert/frag` to use vertex color at location 4
- [x] **Validation**
  - [x] Build succeeds
  - [ ] Grass blocks appear green (runtime)
  - [ ] Dirt blocks appear brown (runtime)
  - [ ] Stone blocks appear gray (runtime)

## Phase 10B: Texture Atlas Infrastructure ✅

- [x] **Texture Array System**
  - [x] Create `core/TextureArray.h/.cpp` for 2D array textures
  - [x] Load individual block textures into array layers
- [x] **Block Texture Assets**
  - [x] Use existing grass_top.png, grass_side.png, dirt.png, stone.png
- [x] **Per-Vertex Texture Index**
  - [x] Replace `color` in `ChunkVertex` with `texIndex`
  - [x] Use `GetTextureIndex()` in mesh builder
- [x] **Shader Updates**
  - [x] Sample from `sampler2DArray` using texture index
  - [x] Combine texture color with lighting
- [x] **Validation**
  - [x] Build succeeds
  - [ ] Block textures visually distinct (runtime)

## Phase 11: Scalable Block & Texture Registry System ✅

### Planning

- [x] Review current architecture (TextureArray, Block.h, Engine.cpp)
- [x] Design BlockRegistry and TextureRegistry classes
- [x] Create implementation plan
- [x] Implementation completed

### Core Infrastructure

- [x] Add nlohmann/json to CMakeLists.txt via FetchContent
- [x] Create `core/TextureRegistry.h/.cpp`
- [x] Create `world/BlockRegistry.h/.cpp`
- [x] Create `assets/config/blocks.json` with block definitions

### Refactoring

- [x] Refactor `Block.h` - Replace enum with BlockID typedef
- [x] Refactor `Chunk.h` - Update block storage to uint16_t
- [x] Refactor `ChunkMeshBuilder.cpp` - Use BlockRegistry for texture lookups
- [x] Refactor `Engine.cpp` - Initialize registries in SetupWorld()
- [x] Refactor `TerrainGenerator.cpp` - Use BlockID from BlockRegistry

### New Blocks

- [x] Add bedrock (Y=0 layer)
- [x] Add sand (beaches)
- [x] Add gravel (rivers, caves)
- [x] Add cobblestone (cave walls)
- [x] Add oak_log, oak_planks, oak_leaves (tree structure)
- [x] Add ores: coal, iron, gold, diamond, copper, emerald

### Validation

- [x] Build succeeds
- [x] Existing terrain renders correctly
- [x] New blocks appear properly textured
- [x] No visual regression from Phase 10B

## Phase 12: Raycasting & Block Interaction ✅

### Core System

- [x] Create `core/Ray.h` - Ray struct with origin/direction
- [x] Create `world/VoxelRaycast.h/.cpp` - DDA algorithm for voxel traversal
- [x] Implement `RaycastResult` struct (hit position, block type, face normal)

### Integration

- [x] Add raycasting to `ChunkManager` for world queries (`GetBlock()`, `SetBlock()`)
- [x] Hook up mouse click events in `Engine.cpp`
- [x] Crosshair UI overlay (`assets/shaders/ui.vert/frag`, `SetupCrosshair()`, `RenderCrosshair()`)
- [x] Visual feedback for targeted block (highlight/outline) - Implemented in Phase 13B

### Block Interaction

- [x] Left-click: Break block (set to Air)
- [x] Right-click: Place block (next to hit face)
- [x] Mesh rebuilding after block changes (async via `RebuildChunkMesh()`)

### Validation

- [x] Build succeeds
- [x] Raycast correctly identifies block under crosshair
- [x] Block breaking works and mesh updates
- [x] Block placing works at correct position

## Phase 13: Visual Enhancements ✅

### Part A: Distance Fog

- [x] Update `lit.frag` and `water.frag` with fog calculation
- [x] Add fog uniforms (FOG_START, FOG_END, FOG_COLOR) to WorldConfig.h
- [x] Fog color matches sky background

### Part B: Block Outline

- [x] Create `BlockOutline.h/.cpp` wireframe cube renderer
- [x] Create `outline.vert/frag` shaders
- [x] Render highlight on targeted block

## Phase 14: Sky System (In Progress)

### Part A: Cloud Layer

- [x] Create `world/SkyRenderer.h/.cpp`
- [x] Create `cloud.vert/frag` shaders
- [x] Render flat cloud plane at Y=192
- [x] Scrolling texture for cloud drift
- [x] Add sky config to `WorldConfig.h`

### Part B: Sun & Moon

- [x] Create `celestial.vert/frag` shaders
- [x] Billboard sun sprite using `sun.png` (32×32)
- [x] Billboard moon with 8 phases from `moon_phases.png` (128×64)
- [x] Sun/moon orbit opposite each other

### Part C: Day/Night Cycle

- [x] Time-of-day system (0.0-1.0 cycle)
- [x] Dynamic sky color (blue → orange → dark blue)
- [x] Dynamic light direction matching sun position
- [x] Adjust ambient light for night

### Part D: Weather System

- [x] Create `weather.vert/frag` shaders
- [x] Rain particles using `rain.png` (64×256)
- [x] Snow particles using `snow.png` (64×256)
- [x] Particle cylinder around player
- [x] Toggle with K key

### Part E: Volumetric Clouds (Fancy Mode)

- [x] Create `volumetric_cloud.vert/frag` shaders
- [x] FastNoiseLite for cloud shape generation
- [x] 3D voxel-like cloud blocks with face culling
- [x] Two-tone lighting (bright tops, shaded sides)

## Phase 15: Shadow Mapping (In Progress)

### Part A: Basic Shadow Map

- [x] Create `core/ShadowMap.h/.cpp` (depth-only FBO)
- [x] Create `shadow.vert/frag` shaders (depth pass)
- [x] Calculate light-space matrix from sun direction
- [x] Render scene from sun's POV for depth

### Part B: Shader Integration

- [x] Update `lit.frag` with shadow sampling
- [x] Add `u_ShadowMap` and `u_LightSpaceMatrix` uniforms
- [x] Implement PCF (Percentage Closer Filtering) for soft shadows
- [x] Update `Engine.cpp` with shadow render pass

### Part C: Cascaded Shadow Maps (CSM)

- [ ] Split view frustum into 3 cascades
- [ ] Multiple shadow maps per cascade (2048, 2048, 1024)
- [ ] Select cascade in fragment shader based on depth
- [ ] Blend between cascades for smooth transitions

### Validation

- [x] Build succeeds
- [ ] Shadows cast correctly at various sun angles
- [ ] No shadow acne or peter-panning artifacts
- [ ] Performance acceptable (>30 FPS)

## Phase 16A: SSAO (Screen-Space Ambient Occlusion) ✅

> Key decisions: Half-res SSAO, Forward + Depth Pre-pass (not full deferred)
> Vertex AO kept: SSAO ON = use SSAO only, SSAO OFF = use vertex AO

### SSAO System

- [x] Create `core/SSAO.h` header
- [x] Create `core/SSAO.cpp` implementation
  - [x] Create depth pre-pass FBO (depth + normal textures)
  - [x] Generate 64-sample hemisphere kernel
  - [x] Generate 4×4 noise texture
  - [x] Create half-res SSAO FBO
  - [x] Create half-res blur FBO
  - [x] Setup fullscreen quad VAO

### Shaders

- [x] Create `assets/shaders/fullscreen.vert` (reusable fullscreen quad)
- [x] Create `assets/shaders/depth_normal.vert` (depth pre-pass)
- [x] Create `assets/shaders/depth_normal.frag` (output view-space normals)
- [x] Create `assets/shaders/ssao.frag` (64-sample hemisphere)
- [x] Create `assets/shaders/ssao_blur.frag` (5×5 box blur)
- [x] Modify `assets/shaders/lit.frag` (either/or AO logic: SSAO or vertex AO)

### Engine Integration

- [x] Add SSAO member to `Engine.h`
- [x] Initialize SSAO in `Engine::SetupWorld()`
- [x] Add depth pre-pass to `ChunkManager` (`RenderAllDepth()`)
- [x] Modify `Engine::Render()` for SSAO passes
  - [x] Depth + normal pre-pass
  - [x] SSAO calculation pass
  - [x] Blur pass
  - [x] Bind SSAO texture for lit.frag
- [x] Add O key toggle for SSAO
- [ ] Handle window resize

### Configuration

- [x] Add SSAO config to `WorldConfig.h`
  - [x] SSAO_ENABLED, SSAO_KERNEL_SIZE, SSAO_RADIUS, SSAO_BIAS, SSAO_POWER

### Validation

- [x] Build succeeds
- [ ] Corners and crevices show darkening
- [ ] No banding or noise artifacts
- [ ] O key toggles SSAO on/off
- [ ] Vertex AO works when SSAO is off
- [ ] Performance acceptable (>30 FPS)
