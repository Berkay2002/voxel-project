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
- **Build System**: CMake 3.30+
- **Windowing/Input**: GLFW 3.4
- **Graphics API**: OpenGL 4.6 (Core Profile)
- **Loaders**: GLAD (or similar)
- **Math**: GLM 1.0.3
- **Noise**: FastNoiseLite (Planned)

## Architecture & Considerations
- **Modular Core**: The engine (`core/`) must be decoupled from the game logic (`game/`).
- **Voxel Data**: Chunks are 16x16x256. stored as 1D/3D arrays.
- **Optimization**:
    - Aggressive Face Culling (never render internal faces).
    - Multithreaded Chunk Generation.
- **Rendering**:
    - Raycasting / DDA for specific checks.
    - Mesh rebuilding loop: Update -> Cull -> Mess -> GPU.

*Refer to `.agent/architecture_reference.md` for the full architectural breakdown.*
