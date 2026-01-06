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

## Building the Project

### Windows (Visual Studio)

**Prerequisites:**

- Visual Studio 2022 (with C++ Desktop Development workload)
- CMake 3.28+ (usually bundled with VS, or install separately)

**Build Steps:**

```powershell
# Configure (generates Visual Studio solution)
cmake -B build -S .

# Build Release configuration
cmake --build build --config Release

# Run the game
.\build\Release\VoxelEngine.exe
```

### Linux

**Prerequisites:**

- GCC 11+ or Clang 14+ (C++20 support)
- CMake 3.28+
- OpenGL development libraries: `sudo apt install libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`

**Build Steps:**

```bash
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run the game
./build/VoxelEngine
```

## Architecture & Considerations

- **Modular Core**: The engine (`core/`) is organized into subdirectories:
  - `core/Application` - Main application loop (formerly Engine)
  - `core/graphics/` - Shader, Texture, TextureArray, VertexArray, VertexBuffer, IndexBuffer
  - `core/rendering/` - ShadowMap, SSAO, SelectionRenderer, TextureManager
  - `core/scene/` - Camera, Frustum, Ray
  - `core/atmosphere/` - SkySystem (clouds, celestials, weather)
  - `core/window/` - Window management
  - `core/settings/` - VideoSettings, GraphicsSettings
- **Centralized Config**: Tunable world parameters in `world/WorldConfig.h`, game config in `world/GameConfig.h`
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
  - Core engine loop: `Window`, `Application`, `Logger`
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
- **Phase 13A**: ✅ Complete (2026-01-04)
  - Distance fog in lit/water shaders
  - Linear fog blending based on camera distance
  - Configurable FOG_START/FOG_END in WorldConfig.h
- **Phase 13B**: ✅ Complete (2026-01-04)
  - Block outline/highlight for targeted block
  - Wireframe cube rendered via BlockOutline class
  - outline.vert/frag shaders
- **Phase 14**: ✅ Complete (2026-01-04)
  - Cloud layer at Y=192 with scrolling texture
  - Sun/moon billboards with 8 moon phases
  - Day/night cycle (20-min, Minecraft default)
  - Dynamic sky color and lighting
  - Rain/snow weather particles (K key toggle)
  - SkySystem class with modular subsystems
- **Phase 13C**: 🔜 Backlog (Greedy Meshing LOD)
  - Reduced vertex count for distant chunks

### Core Refactoring (2026-01-06)

- **Phase R1**: ✅ Complete - File Organization
  - Organized `core/` into subdirectories: graphics/, rendering/, scene/, atmosphere/, window/, settings/
- **Phase R2**: ✅ Complete - Class Renaming
  - Engine → Application, SkyRenderer → SkySystem
  - DisplayConfig → VideoSettings, RenderConfig → GraphicsSettings
- **Phase R3**: ✅ Complete - Naming Conventions
  - Applied Unreal-style `m_b` prefix to boolean members

### Performance Optimizations (2026-01-06)

- **Phase 17**: ✅ Complete - Core Rendering Optimizations
  - SSAO uniform location caching (no per-frame string allocs)
  - Pre-computed chunk model matrices across 4 render passes
  - SSAO kernel UBO (single buffer bind instead of 64 uniforms)

### Weather System Improvements (2026-01-06)

- **Phase 18**: ✅ Complete - Heightmap-Based Weather Occlusion
  - Minecraft-style heightmap system (16×16 per chunk)
  - Per-chunk heightmap tracking (highest solid block per column)
  - ChunkManager heightmap queries for weather occlusion
  - 3×3 grid sampling around player for robust occlusion
  - Weather shader spawns particles above terrain height
  - Smooth fade-out as particles approach ground (3-0.5 blocks)
  - Automatic culling of underground/cave particles
  - No weather rendering when player is indoors/underground
  - Centralized weather configuration in WorldConfig.h
  - Comprehensive testing guide (WEATHER_TESTING.md)
