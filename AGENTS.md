## AI Agent Instructions

- Start every task by reading [.agent/task.md](.agent/task.md) (index) plus the relevant file under [.agent/tasks](.agent/tasks); keep task checkboxes updated where the work lives.
- Engine loop lives in [core/Application.h](core/Application.h) and drives input, camera, chunk manager, and render passes (opaque → water → outlines → sky/weather); avoid touching GL from worker threads.
- World model: chunks are 16×16×256 blocks stored in `uint16_t` IDs; mesh generation happens in [world/ChunkMeshBuilder.cpp](world/ChunkMeshBuilder.cpp) using aggressive face culling and returns separate opaque/water meshes.
- Registry-driven blocks/textures: [world/BlockRegistry.cpp](world/BlockRegistry.cpp) and [core/TextureRegistry.cpp](core/TextureRegistry.cpp) load [assets/config/blocks.json](assets/config/blocks.json) and texture layers; prefer registry lookups over hardcoded enums.
- Terrain/caves: [world/TerrainGenerator.cpp](world/TerrainGenerator.cpp) combines biome noise, rivers, and cave carvers (see [world/SpaghettiCaveCarver.cpp](world/SpaghettiCaveCarver.cpp)); all tunables centralized in [world/WorldConfig.h](world/WorldConfig.h).
- Async loading: chunk generation and mesh building run on BS::thread_pool (background); GPU uploads are main-thread only via `ProcessPendingMeshes` in [world/ChunkManager.cpp](world/ChunkManager.cpp) with a capped uploads-per-frame rule.
- Rendering resources: shaders live under [assets/shaders](assets/shaders) (notably `lit`, `water`, `outline`, `weather`, `volumetric_cloud`, `ssao`); TextureArray is in [core/graphics/TextureArray.cpp](core/graphics/TextureArray.cpp) and is the default path for block textures.
- Lighting & post: directional sun + AO in `lit` shader pair; SSAO path uses precomputed kernel UBO and cached uniform locations; fog parameters are in `WorldConfig` and applied in lit/water shaders.
- Interaction: raycast DDA in [world/VoxelRaycast.cpp](world/VoxelRaycast.cpp) feeds block break/place; block outline rendering is in [core/rendering/SelectionRenderer](core/rendering) (outline shaders) and UI crosshair in [ui/UIRenderer.cpp](ui/UIRenderer.cpp).
- Weather & sky: [core/atmosphere/SkySystem.cpp](core/atmosphere/SkySystem.cpp) orchestrates sun/moon, clouds, and weather particles; weather spawns respect per-chunk heightmaps from [world/ChunkManager.cpp](world/ChunkManager.cpp) and config in [WEATHER_IMPLEMENTATION.md](WEATHER_IMPLEMENTATION.md).
- Configuration defaults: global gameplay/video settings in [world/GameConfig.h](world/GameConfig.h) and [core/settings](core/settings); avoid scattering constants—extend `WorldConfig`/`GameConfig` instead.
- Naming/style: modern C++20; booleans use `m_b` prefix; prefer `glm` types for math; avoid raw OpenGL calls outside graphics/rendering files unless following existing patterns.
- Build/run: `cmake -B build -S . && cmake --build build -j$(nproc)`; run with `./build/VoxelEngine`; tests via `cmake --build build --target VoxelTests` then `./build/tests/VoxelTests`.
- Assets are large; do not embed binaries in commits—reference existing textures under [assets/textures](assets/textures) and keep new art external unless requested.
- When adding features that touch multiple systems (world gen, meshing, rendering), update both registries and configs, and ensure mesh data remains CPU-only on worker threads.
