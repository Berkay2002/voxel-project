# GEMINI.md / Project Context

## The `.agent` Folder

The `.agent` directory serves as the persistent "brain" and memory for AI agents working on this project. It contains:

- **`task.md`**: The source of truth for current progress and active tasks.
- **`plans/`**: Detailed implementation plans (e.g., `phase1_plan.md`).
- **`ideas.md`**: Brainstorming notes and future features.
- **`architecture_reference.md`**: Textual representation of the core architecture (derived from `voxel.jpg`).

**Rule for Agents**: Always consult `.agent/task.md` and `.agent/architecture_reference.md` before starting new tasks.

**Rule for Implementation**: During and after implementation phases:

1. Update `.agent/task.md` with progress (`[ ]` → `[/]` → `[x]`)
2. Update `GEMINI.md` Project Status when phases complete
3. Keep implementation plans in `.agent/plans/` for reference

## Project Overview

**Voxel Project** is a custom C++ game engine tailored for voxel-based rendering and gameplay. The goal is to build a performant, modular engine from scratch using modern C++.

## Tech Stack

- **Language**: C++20
- **Build System**: CMake 3.28+
- **Windowing/Input**: GLFW 3.4
- **Graphics API**: OpenGL 4.6 (Core Profile)
- **Loaders**: GLAD (or similar)
- **Math**: GLM 1.0.1
- **Noise**: FastNoiseLite 1.1.1

## Architecture & Considerations

- **Modular Core**: The engine (`core/`) must be decoupled from the game logic (`world/`).
- **Centralized Config**: All tunable world parameters are in `world/WorldConfig.h`.
- **Voxel Data**: Chunks are 16x16x256, stored as 1D/3D arrays.
- **Optimization**:
  - Aggressive Face Culling (never render internal faces).
  - Multithreaded Chunk Generation.
- **Rendering**:
  - Raycasting / DDA for specific checks.
  - Mesh rebuilding loop: Update -> Cull -> Mesh -> GPU.

_Refer to `.agent/architecture_reference.md` for the full architectural breakdown._

## Project Status

- **Phase 1**: ✅ Complete (2026-01-03)
  - CMake build system with FetchContent (GLFW 3.4, GLAD, GLM 1.0.1)
  - Core engine loop: `Window`, `Engine`, `Logger`
  - OpenGL 4.6 context verified (tested on RTX 3090)
- **Phase 2**: ✅ Complete (2026-01-03)
  - Shader system with uniform setters
  - Buffer abstractions: VAO, VBO, EBO
  - Texture system with stb_image
  - Camera with WASD + mouse look
- **Phase 3A**: ✅ Complete (2026-01-03)
  - Block types: Air, Dirt, Grass, Stone
  - Chunk data structure (16×16×256)
  - Mesh generation with face culling
  - Single chunk rendering validated
- **Phase 3B**: ✅ Complete (2026-01-03)
  - FastNoiseLite integration via FetchContent
  - TerrainGenerator with Perlin noise
  - Procedural hills with Stone/Dirt/Grass layering
- **Phase 4**: ✅ Complete (2026-01-03)
  - ChunkManager with hash map storage
  - Per-chunk GPU mesh resources
  - 5×5 chunk loading around player
  - Seamless terrain across chunk boundaries
- **Phase 5**: ✅ Complete (2026-01-03)
  - Frustum culling (Gribb-Hartmann plane extraction, AABB tests)
  - BS::thread_pool for async chunk generation
  - Separated mesh generation (worker) from GPU upload (main thread)
  - Rate-limited uploads (2 per frame) to avoid hitches
- **Phase 6**: ✅ Complete (2026-01-03)
  - Directional sun lighting with ambient light
  - Per-vertex ambient occlusion
- **Phase 7**: ✅ Complete (2026-01-03)
  - Water BlockType with transparent rendering
  - Separate opaque/water mesh generation
  - Two-pass rendering with alpha blending
  - Water shader with blue tint
- **Phase 8A**: ✅ Complete (2026-01-03)
  - Modular `ICaveCarver` interface for multiple cave types
  - `SpaghettiCaveCarver` with 3D Perlin noise (Minecraft-style tunnels)
  - Two-pass terrain generation (terrain + cave carving)
  - Water flooding for underwater caves
  - Natural hillside cave entrances (surfaceProtection=0)
  - Centralized `WorldConfig.h` for all tunable parameters
- **Phase 8B**: 🔜 Backlog (More Caves & World Features)
  - Cheese caves (large caverns)
  - Noodle caves (thin connecting tunnels)
  - More block types (Sand, Gravel, Cobblestone)
  - Texture atlas for multiple block textures
- **Phase 9**: ✅ Complete (2026-01-03)
  - Biome system: Plains (flat, Y≈45) and Mountains (dramatic, Y≈65+)
  - Biome noise for large-scale region selection
  - River water using cellular noise (winding paths in Plains only)
  - Removed global sea level flooding
- **Phase 10A**: ✅ Complete (2026-01-03)
  - Per-vertex block colors (Grass=green, Dirt=brown, Stone=gray)
  - `GetBlockColor()` helper in `Block.h`
  - Updated shaders to use vertex color attribute (location 4)
- **Phase 10B**: ✅ Complete (2026-01-04)
  - `TextureArray` class for GL_TEXTURE_2D_ARRAY
  - Per-vertex texture index (`texIndex`) in `ChunkVertex`
  - Shaders sample from `sampler2DArray` using layer index
  - Block textures: grass_top, dirt, grass_side, stone
- **Phase 11**: ✅ Complete (2026-01-04)
  - Scalable Block & Texture Registry System
  - Data-driven `blocks.json` configuration
  - `BlockRegistry` and `TextureRegistry` singletons
  - 18 block types with per-face textures
  - New blocks: bedrock, sand, gravel, cobblestone, ores, wood
- **Phase 12**: ✅ Complete (2026-01-04)
  - DDA algorithm for voxel ray traversal
  - Block breaking (left-click) and placing (right-click)
  - Async mesh rebuilding after block modifications
  - Cross-chunk boundary handling
  - Crosshair UI overlay (Minecraft-style + at screen center)
