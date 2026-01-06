# Phase 16B+: Advanced & Backlog

## Phase 16B: Compute Shader Mesh Generation (Pending)

> GPU-based mesh generation using compute shaders and SSBOs
> Keeps vertex AO calculation (matches CPU path)

### ComputeMesher Class

- [ ] Create `core/ComputeMesher.h` header
- [ ] Create `core/ComputeMesher.cpp` implementation
  - [ ] Setup Block Data SSBO (65536 × uint16_t)
  - [ ] Setup Opaque Vertex Output SSBO
  - [ ] Setup Water Vertex Output SSBO
  - [ ] Setup atomic counter buffer
  - [ ] Setup Block Lookup Table SSBO (per-block textures/flags)

### Compute Shader

- [ ] Create `assets/shaders/mesh_gen.comp`
  - [ ] Read block data from SSBO
  - [ ] Face culling for all 6 directions
  - [ ] Vertex AO calculation (24-neighbor lookup)
  - [ ] Emit quads with position, UV, normal, AO, texIndex, tint
  - [ ] Separate opaque and water output
  - [ ] Handle chunk boundary blocks

### ChunkManager Integration

- [ ] Add `ComputeMesher` member to `ChunkManager`
- [ ] Add `m_UseComputeMeshing` toggle
- [ ] Modify `LoadChunkAsync()` for GPU path
  - [ ] Upload block data to SSBO
  - [ ] Dispatch compute shader
- [ ] Modify `ProcessPendingMeshes()` for compute results
  - [ ] Read atomic counter for vertex count
  - [ ] Copy SSBO to chunk VBO
- [ ] Add G key toggle for CPU/GPU meshing

### Configuration

- [ ] Add compute meshing config to `WorldConfig.h`

### Validation

- [ ] Build succeeds
- [ ] Meshes identical to CPU-generated
- [ ] No missing faces
- [ ] G key toggles CPU/GPU meshing
- [ ] Performance improvement measurable

---

## Phase 16C: Compute Shader Terrain Generation (Backlog)

> Deferred due to GPU/CPU noise precision concerns
> Risk: chunk boundary artifacts if GPU noise doesn't match FastNoiseLite exactly

### If Implemented Later

- [ ] Create `world/ComputeTerrainGenerator.h/.cpp`
- [ ] Implement GPU Perlin noise in GLSL
- [ ] Implement GPU Cellular noise in GLSL
- [ ] Create `assets/shaders/terrain_gen.comp`
- [ ] Create `assets/shaders/cave_carve.comp`
- [ ] Create `assets/shaders/ore_gen.comp`
- [ ] Validate output matches CPU FastNoiseLite exactly
- [ ] Test chunk boundary alignment

---

## Phase 17: Voxel Light Propagation (Backlog)

### Part A: Skylight System

- [ ] Create `world/LightMap.h` (per-chunk light storage)
- [ ] Add `LightMap` member to `Chunk`
- [ ] Implement skylight flood-fill from top
- [ ] Light level 15 at top, decreases by 1 per block

### Part B: Block Light System

- [ ] Add `lightLevel` property to block definitions
- [ ] Create light-emitting blocks (torch, glowstone)
- [ ] Implement BFS propagation for block light
- [ ] Handle chunk boundary propagation

### Part C: Shader Integration

- [ ] Add `lightLevel` attribute to `ChunkVertex`
- [ ] Update `ChunkMeshBuilder` to sample light map
- [ ] Update `lit.vert/frag` to use light level
- [ ] Smooth light interpolation between blocks

### Validation

- [ ] Build succeeds
- [ ] Underground caves are dark
- [ ] Torches illuminate surrounding blocks
- [ ] Light propagates across chunk boundaries

---

## Future: RTX Migration (Research)

- [ ] Evaluate Vulkan vs OptiX for ray tracing
- [ ] Document OpenGL → Vulkan migration path
- [ ] Prototype ray-traced shadows with VK_KHR_ray_tracing
- [ ] Research AI denoising (OptiX AI Denoiser)

---

## Phase 18: Documentation & Polish ✅

- [x] **Project Identity**
  - [x] Generate Project Logo
  - [x] Create `logo.png` in root
- [x] **Documentation**
  - [x] Create `README.md`
  - [x] Add build instructions, features, and tech stack
  - [x] Include infographic and logo
