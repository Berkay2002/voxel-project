# Core Refactoring Plan: AAA Industry Standards

This plan reorganizes `core/` to follow AAA game engine conventions (Unreal, id Tech, Frostbite patterns).

## Decisions Made

| Decision           | Choice                                 | Rationale                                                 |
| ------------------ | -------------------------------------- | --------------------------------------------------------- |
| Nested namespaces  | **No** — keep flat `Core::`            | Project size doesn't justify complexity                   |
| Split SkyRenderer  | **Defer** — rename to `SkySystem` only | 951 lines is manageable; split when adding major features |
| Scope              | **`core/` only**                       | Foundation first; apply lessons to `world/` later         |
| Boolean convention | **`m_bEnabled`** (Unreal-style)        | Industry standard, shorter than `m_isEnabled`             |

---

## ✅ Phase 1: File Organization (COMPLETED 2026-01-06)

### Commits:

1. `9904386` - `core/graphics/` (Shader, Texture, TextureArray, VertexArray, VertexBuffer, IndexBuffer)
2. `ce7f607` - `core/rendering/` (ShadowMap, SSAO, SelectionRenderer, TextureManager)
3. `99071d2` - `core/scene/` (Camera, Frustum, Ray)
4. `896ec8f` - `core/atmosphere/`, `core/window/`, `core/settings/`

### Current Structure (After Phase 1)

```
core/
├── atmosphere/
│   └── SkySystem.cpp/h        # File renamed (class still SkyRenderer)
├── graphics/
│   ├── Shader.cpp/h
│   ├── Texture.cpp/h
│   ├── TextureArray.cpp/h
│   ├── VertexArray.cpp/h
│   ├── VertexBuffer.cpp/h
│   └── IndexBuffer.cpp/h
├── rendering/
│   ├── SelectionRenderer.cpp/h  # ✓ Fully renamed from BlockOutline
│   ├── SSAO.cpp/h
│   ├── ShadowMap.cpp/h
│   └── TextureManager.cpp/h     # ✓ Fully renamed from TextureRegistry
├── scene/
│   ├── Camera.cpp/h
│   ├── Frustum.cpp/h
│   └── Ray.h
├── settings/
│   ├── VideoSettings.cpp/h      # File renamed (class still DisplayConfig)
│   └── GraphicsSettings.cpp/h   # File renamed (class still RenderConfig)
├── window/
│   └── Window.cpp/h
├── Engine.cpp/h                 # To be renamed Application in Phase 2
├── GameState.h
└── Logger.h
```

### Namespace Strategy

- **Keep flat `Core::` namespace** for all classes
- Subdirectories are for file organization only, not namespace hierarchy
- Example: `core/graphics/Shader.h` still uses `namespace Core { class Shader; }`

---

## 🔜 Phase 2: Internal Class Renaming (NEXT)

Files were moved/renamed in Phase 1, but some internal class names still need updating.

### Class Renaming Required

| File                                     | Current Class       | Target Class       | Status                |
| ---------------------------------------- | ------------------- | ------------------ | --------------------- |
| `core/Engine.cpp/h`                      | `Engine`            | `Application`      | Pending + file rename |
| `core/atmosphere/SkySystem.cpp/h`        | `SkyRenderer`       | `SkySystem`        | Pending               |
| `core/settings/VideoSettings.cpp/h`      | `DisplayConfig`     | `VideoSettings`    | Pending               |
| `core/settings/GraphicsSettings.cpp/h`   | `RenderConfig`      | `GraphicsSettings` | Pending               |
| `core/rendering/SelectionRenderer.cpp/h` | `SelectionRenderer` | -                  | ✅ Complete           |
| `core/rendering/TextureManager.cpp/h`    | `TextureManager`    | -                  | ✅ Complete           |

### Implementation Steps

#### Step 2.1: Engine → Application

```bash
git mv core/Engine.cpp core/Application.cpp
git mv core/Engine.h core/Application.h
```

Then inside files:

- Replace `class Engine` → `class Application`
- Replace all references across codebase

#### Step 2.2: SkyRenderer → SkySystem

Inside `core/atmosphere/SkySystem.cpp/h`:

- Replace `class SkyRenderer` → `class SkySystem`
- Replace all constructor/destructor names
- Update forward declarations in Engine.h/Application.h
- Update usages in Engine.cpp/Application.cpp

#### Step 2.3: DisplayConfig → VideoSettings

Inside `core/settings/VideoSettings.cpp/h`:

- Replace `class DisplayConfig` → `class VideoSettings`
- Update usages in Window.cpp/h, SettingsScreen.cpp

#### Step 2.4: RenderConfig → GraphicsSettings

Inside `core/settings/GraphicsSettings.cpp/h`:

- Replace `class RenderConfig` → `class GraphicsSettings`
- Update usages in SettingsScreen.cpp

---

## 🔜 Phase 3: Code Naming Conventions

### 3.1 Boolean Naming (`m_b` prefix)

Apply to all boolean members across `core/`:

```cpp
// Before                          // After
bool m_CloudsEnabled;        →     bool m_bCloudsEnabled;
bool m_CelestialsEnabled;    →     bool m_bCelestialsEnabled;
bool m_WeatherEnabled;       →     bool m_bWeatherEnabled;
bool m_FirstMouse;           →     bool m_bFirstMouse;
bool m_CursorCaptured;       →     bool m_bCursorCaptured;
bool m_WorldSetupStarted;    →     bool m_bWorldSetupStarted;
bool m_Enabled;              →     bool m_bEnabled;  // In SSAO
```

### 3.2 Constants Header (Optional)

Create `core/Constants.h`:

```cpp
#pragma once

namespace Core {

// Camera defaults
constexpr float kDefaultFOV = 45.0f;
constexpr float kDefaultNearPlane = 0.1f;
constexpr float kDefaultFarPlane = 500.0f;
constexpr float kDefaultMoveSpeed = 5.0f;
constexpr float kDefaultSprintMultiplier = 2.5f;
constexpr float kDefaultMouseSensitivity = 0.1f;

// Rendering
constexpr int kShadowMapResolution = 4096;
constexpr int kSSAOKernelSize = 64;
constexpr int kSSAONoiseSize = 4;

// Application
constexpr int kDefaultWindowWidth = 1280;
constexpr int kDefaultWindowHeight = 720;

} // namespace Core
```

---

## Verification Plan

### Build Verification

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/VoxelEngine
```

### Feature Verification Checklist

- [ ] Game launches to title screen
- [ ] Loading screen works
- [ ] World renders correctly
- [ ] Day/night cycle works (clouds, sun, moon)
- [ ] Weather toggle (K key) works
- [ ] SSAO toggle (O key) works
- [ ] Shadows render correctly
- [ ] Block interaction works (break/place)
- [ ] Block outline shows on hover
- [ ] Settings screen opens/closes
- [ ] Window resize works

### Documentation Update

- [ ] Update `GEMINI.md` with new file structure
- [ ] Update `.agent/architecture_reference.md` if needed

---

## Progress Summary

| Phase | Description             | Status      | Date       |
| ----- | ----------------------- | ----------- | ---------- |
| 1     | File Organization       | ✅ Complete | 2026-01-06 |
| 2     | Internal Class Renaming | 🔜 Next     | -          |
| 3     | Code Naming Conventions | Pending     | -          |
