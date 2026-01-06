# Core Refactoring Task

Comprehensive refactoring of `core/` directory to follow AAA game engine standards.

**Plan**: [core_refactoring_plan.md](file:///home/berkay-orhan/Developer/playground/voxel-project/.agent/plans/core_refactoring_plan.md)

## Key Decisions

- **Namespace**: Keep flat `Core::` (no nesting)
- **SkyRenderer**: Rename to `SkySystem`, defer splitting
- **Scope**: `core/` only (defer `world/` and `ui/`)
- **Booleans**: Use `m_b` prefix (Unreal-style)

---

## ✅ Phase 1: File Organization (COMPLETED 2026-01-06)

### Commits Made:

1. `9904386` - Move graphics files to `core/graphics/`
2. `ce7f607` - Move rendering files to `core/rendering/`, rename BlockOutline→SelectionRenderer, TextureRegistry→TextureManager
3. `99071d2` - Move scene files to `core/scene/`
4. `896ec8f` - Move remaining files to `core/atmosphere/`, `core/window/`, `core/settings/`

### Completed Tasks:

- [x] Create subdirectory structure
  - [x] `core/graphics/`
  - [x] `core/rendering/`
  - [x] `core/scene/`
  - [x] `core/atmosphere/`
  - [x] `core/window/`
  - [x] `core/settings/`
- [x] Move files to `graphics/`: Shader, Texture, TextureArray, VertexArray, VertexBuffer, IndexBuffer
- [x] Move files to `rendering/`: ShadowMap, SSAO, BlockOutline→SelectionRenderer, TextureRegistry→TextureManager
- [x] Move files to `scene/`: Camera, Frustum, Ray
- [x] Move files to `atmosphere/`: SkyRenderer→SkySystem (file renamed, class name pending)
- [x] Move files to `window/`: Window
- [x] Move files to `settings/`: DisplayConfig→VideoSettings, RenderConfig→GraphicsSettings (files renamed, class names pending)
- [x] Update CMakeLists.txt with new paths
- [x] Update all `#include` paths across `core/`, `world/`, `ui/`
- [ ] ~~Rename `Engine` → `Application`~~ (deferred to Phase 2)

### Current File Structure:

```
core/
├── atmosphere/
│   └── SkySystem.cpp/h        # Class still named SkyRenderer (rename in Phase 2)
├── graphics/
│   ├── Shader.cpp/h
│   ├── Texture.cpp/h
│   ├── TextureArray.cpp/h
│   ├── VertexArray.cpp/h
│   ├── VertexBuffer.cpp/h
│   └── IndexBuffer.cpp/h
├── rendering/
│   ├── SelectionRenderer.cpp/h  # ✓ Class renamed from BlockOutline
│   ├── SSAO.cpp/h
│   ├── ShadowMap.cpp/h
│   └── TextureManager.cpp/h     # ✓ Class renamed from TextureRegistry
├── scene/
│   ├── Camera.cpp/h
│   ├── Frustum.cpp/h
│   └── Ray.h
├── settings/
│   ├── VideoSettings.cpp/h      # Class still named DisplayConfig (rename in Phase 2)
│   └── GraphicsSettings.cpp/h   # Class still named RenderConfig (rename in Phase 2)
├── window/
│   └── Window.cpp/h
├── Engine.cpp/h                 # Rename to Application in Phase 2
├── GameState.h
└── Logger.h
```

---

## 🔜 Phase 2: Internal Class Renaming (NEXT)

**Prerequisites**: Phase 1 complete, all builds pass.

### Class Renaming (files already renamed, need internal changes):

| File Location                          | Current Class Name | New Class Name     |
| -------------------------------------- | ------------------ | ------------------ |
| `core/Engine.cpp/h`                    | `Engine`           | `Application`      |
| `core/atmosphere/SkySystem.cpp/h`      | `SkyRenderer`      | `SkySystem`        |
| `core/settings/VideoSettings.cpp/h`    | `DisplayConfig`    | `VideoSettings`    |
| `core/settings/GraphicsSettings.cpp/h` | `RenderConfig`     | `GraphicsSettings` |

### Tasks:

- [ ] Rename `Engine` class to `Application` (+ rename files)
- [ ] Rename `SkyRenderer` class to `SkySystem` inside atmosphere/SkySystem.cpp/h
- [ ] Rename `DisplayConfig` class to `VideoSettings` inside settings/VideoSettings.cpp/h
- [ ] Rename `RenderConfig` class to `GraphicsSettings` inside settings/GraphicsSettings.cpp/h
- [ ] Update all usages across codebase (forward declarations, member types, etc.)
- [ ] Remove backwards-compatibility aliases after all usages updated

### Files to Update (per class rename):

**Engine → Application:**

- `core/Engine.h` → `core/Application.h` (file rename + class rename)
- `core/Engine.cpp` → `core/Application.cpp`
- `main.cpp` (creates Engine instance)
- All forward declarations in other headers

**SkyRenderer → SkySystem:**

- `core/atmosphere/SkySystem.h` (class declaration)
- `core/atmosphere/SkySystem.cpp` (class implementation)
- `core/Engine.h` (forward declaration + member type)
- `core/Engine.cpp` (usages)

**DisplayConfig → VideoSettings:**

- `core/settings/VideoSettings.h` (class declaration)
- `core/settings/VideoSettings.cpp` (class implementation)
- `core/window/Window.h` (includes VideoSettings.h, uses DisplayConfig)
- `core/window/Window.cpp` (uses DisplayConfig)
- `ui/SettingsScreen.cpp` (uses DisplayConfig)

**RenderConfig → GraphicsSettings:**

- `core/settings/GraphicsSettings.h` (class declaration)
- `core/settings/GraphicsSettings.cpp` (class implementation)
- `ui/SettingsScreen.cpp` (uses RenderConfig)

---

## 🔜 Phase 3: Code Naming Conventions

**Prerequisites**: Phase 2 complete.

- [ ] Add `m_b` prefix to boolean members in:
  - [ ] Application.h (m_FirstMouse→m_bFirstMouse, m_CursorCaptured→m_bCursorCaptured, m_WorldSetupStarted→m_bWorldSetupStarted)
  - [ ] SkySystem.h (m_CloudsEnabled→m_bCloudsEnabled, m_CelestialsEnabled→m_bCelestialsEnabled, m_WeatherEnabled→m_bWeatherEnabled)
  - [ ] SSAO.h (m_Enabled→m_bEnabled)
  - [ ] Any other files with boolean members
- [ ] Create `core/Constants.h` with centralized magic numbers (optional)
- [ ] Rename generic shader members to purpose-first names (optional)

---

## Verification

### Build Status:

- [x] Build passes: `cmake --build build` ✅ (verified after Phase 1)

### Feature Testing (after Phase 2 complete):

- [ ] Game launches to title screen
- [ ] World renders correctly
- [ ] Day/night cycle works
- [ ] Weather toggle (K) works
- [ ] SSAO toggle (O) works
- [ ] Shadows render correctly
- [ ] Block interaction works
- [ ] Settings screen works

### Documentation:

- [ ] Update GEMINI.md with new structure
