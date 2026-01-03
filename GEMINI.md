# GEMINI.md / Project Context

## The `.agent` Folder

The `.agent` directory serves as the persistent "brain" and memory for AI agents working on this project. It contains:

- **`task.md`**: The source of truth for current progress and active tasks.
- **`plans/`**: Detailed implementation plans (e.g., `phase1_plan.md`).
- **`ideas.md`**: Brainstorming notes and future features.
- **`architecture_reference.md`**: Textual representation of the core architecture (derived from `voxel.jpg`).

**Rule for Agents**: Always consult `.agent/task.md` and `.agent/architecture_reference.md` before starting new tasks.

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

- **Modular Core**: The engine (`core/`) must be decoupled from the game logic (`game/`).
- **Voxel Data**: Chunks are 16x16x256. stored as 1D/3D arrays.
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
- **Phase 5**: 🔜 Next (Multithreading, frustum culling, water)
