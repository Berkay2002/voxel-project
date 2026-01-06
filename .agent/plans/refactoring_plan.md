# Voxel Engine Refactoring Plan

This plan addresses code organization, configuration redundancy, and modularity improvements.

## User Review Required

> [!IMPORTANT] > **SkyRenderer Migration**: Moving `SkyRenderer` from `world/` to `core/` is a significant structural change. It makes sense architecturally (pure rendering, no voxel logic), but may affect your mental model of the codebase.

> [!IMPORTANT] > **Configuration Consolidation**: The current setup has 4 config files with overlapping responsibilities. This plan proposes a cleaner 3-config approach.

---

## Current Issues

### 1. Redundant RuntimeConfig

- `world/RuntimeConfig.h/.cpp` contains the **exact same fields** as `core/RenderConfig.h/.cpp` + `world/GameConfig.h/.cpp`
- This creates confusion - which one should code use?
- RuntimeConfig is effectively dead code (only 1 reference in its own .cpp file)

### 2. Mixed Layer Responsibilities

- `SkyRenderer` is in `world/` but is **pure rendering code** (shaders, meshes, textures)
- It doesn't touch voxel data, chunks, or world logic
- Other pure renderers (`BlockOutline`, `SSAO`, `ShadowMap`) are correctly in `core/`

### 3. Namespace Inconsistency

| File           | Current Namespace | Expected                           |
| -------------- | ----------------- | ---------------------------------- |
| `GameConfig`   | `Voxel::Config::` | `Voxel::Config::` ✅               |
| `RenderConfig` | `Core::`          | `Core::` ✅                        |
| `WorldConfig`  | `Voxel::Config::` | `Voxel::Config::` ✅               |
| `SkyRenderer`  | `Voxel::`         | Should be `Core::` after migration |

---

## Proposed Changes

### Phase 1: Remove Redundant RuntimeConfig

#### [DELETE] `world/RuntimeConfig.h`

- This file is a duplicate of RenderConfig + GameConfig combined
- Currently unused (only 1 self-reference in `.cpp`)

#### [DELETE] `world/RuntimeConfig.cpp`

- Implementation file for redundant RuntimeConfig

#### [MODIFY] `world/GameConfig.h`

- Remove the backwards-compatibility alias: `using RuntimeConfig = GameConfig;`
- This alias is confusing and encourages use of the deprecated name

---

### Phase 2: Migrate SkyRenderer to Core

#### [MOVE] `world/SkyRenderer.h` → `core/SkyRenderer.h`

- Change namespace from `Voxel::` to `Core::`
- Update forward declarations

#### [MOVE] `world/SkyRenderer.cpp` → `core/SkyRenderer.cpp`

- Change namespace from `Voxel::` to `Core::`
- Update include path

#### [MODIFY] `core/Engine.h`

- Change forward declaration from `Voxel::SkyRenderer` to `Core::SkyRenderer`
- Update member type

#### [MODIFY] `core/Engine.cpp`

- Update include from `"world/SkyRenderer.h"` to `"core/SkyRenderer.h"`
- Update any `Voxel::SkyRenderer` references to `Core::SkyRenderer`

#### [MODIFY] `CMakeLists.txt`

- Move SkyRenderer source files from world/ to core/ in source lists

---

### Phase 3: Configuration Summary (After Refactoring)

After these changes, the configuration hierarchy will be clean:

| Config          | Location | Purpose                                                        |
| --------------- | -------- | -------------------------------------------------------------- |
| `WorldConfig.h` | `world/` | **Compile-time** constants (terrain, caves, lighting defaults) |
| `RenderConfig`  | `core/`  | **Runtime** rendering settings (fog, shadows, SSAO, clouds)    |
| `GameConfig`    | `world/` | **Runtime** game settings (day duration, weather toggle)       |
| `DisplayConfig` | `core/`  | **Runtime** window settings (mode, vsync, resolution)          |

---

## Verification Plan

### Automated Build Test

```bash
rm -rf build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Manual Runtime Test

1. Run `./build/VoxelEngine`
2. Verify sky system works (clouds, sun/moon orbit)
3. Press O to toggle SSAO, K to toggle weather
4. Access settings menu - sliders should work

### Regression Checklist

- [ ] No linker errors
- [ ] No include path errors
- [ ] Sky renders correctly
- [ ] Settings screen works
- [ ] No crashes on startup/shutdown
