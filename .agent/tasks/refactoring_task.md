# Refactoring Tasks

## Configuration Cleanup

- [x] Remove redundant `world/RuntimeConfig.h/.cpp` (duplicates RenderConfig + GameConfig)
- [x] Consolidate configuration singletons to eliminate overlap
- [x] Update any remaining references to RuntimeConfig

## Core/World Layer Separation

- [x] Migrate `SkyRenderer` from `world/` to `core/` (pure rendering, no world logic)
- [x] Migrate `BlockOutline` namespace from `Core` (already correct) - verify consistency
- [x] Evaluate `VoxelRaycast` location → **Keep in `world/`** (depends on ChunkManager, BlockRegistry)

## Namespace Consistency

- [x] Standardize namespaces: `Core::` for engine, `Voxel::` for game logic
- [x] Fix mixed namespace usage in config classes (`Voxel::Config::` vs `Core::`)

## Code Quality

- [x] Remove deprecated alias `using RuntimeConfig = GameConfig` in GameConfig.h
- [x] Clean up unused includes and forward declarations → **Audit complete, no issues found**
