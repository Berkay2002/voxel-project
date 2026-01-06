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

## Phase 1: File Organization

### Current Structure (Flat)

```
core/
├── BlockOutline.cpp/h
├── Camera.cpp/h
├── DisplayConfig.cpp/h
├── Engine.cpp/h
├── Frustum.cpp/h
├── GameState.h
├── IndexBuffer.cpp/h
├── Logger.h
├── Ray.h
├── RenderConfig.cpp/h
├── Shader.cpp/h
├── ShadowMap.cpp/h
├── SkyRenderer.cpp/h
├── SSAO.cpp/h
├── Texture.cpp/h
├── TextureArray.cpp/h
├── TextureRegistry.cpp/h
├── VertexArray.cpp/h
├── VertexBuffer.cpp/h
└── Window.cpp/h
```

### Proposed Structure (Grouped by Responsibility)

```
core/
├── Application.cpp/h              # Renamed from Engine
├── GameState.h                    # Application states enum
├── Logger.h                       # Logging utilities
│
├── graphics/                      # Low-level GPU abstractions
│   ├── Shader.cpp/h
│   ├── Texture.cpp/h
│   ├── TextureArray.cpp/h
│   ├── VertexArray.cpp/h
│   ├── VertexBuffer.cpp/h
│   └── IndexBuffer.cpp/h
│
├── rendering/                     # High-level rendering systems
│   ├── ShadowMap.cpp/h
│   ├── SSAO.cpp/h
│   ├── SelectionRenderer.cpp/h   # Renamed from BlockOutline
│   └── TextureManager.cpp/h      # Renamed from TextureRegistry
│
├── scene/                         # Camera, frustum, spatial
│   ├── Camera.cpp/h
│   ├── Frustum.cpp/h
│   └── Ray.h
│
├── atmosphere/                    # Sky, weather, celestials
│   └── SkySystem.cpp/h           # Renamed from SkyRenderer (kept as single file)
│
├── window/                        # Platform/windowing
│   └── Window.cpp/h
│
└── settings/                      # Configuration
    ├── VideoSettings.cpp/h       # Renamed from DisplayConfig
    └── GraphicsSettings.cpp/h    # Renamed from RenderConfig
```

### Namespace Strategy

- **Keep flat `Core::` namespace** for all classes
- Subdirectories are for file organization only, not namespace hierarchy
- Example: `core/graphics/Shader.h` still uses `namespace Core { class Shader; }`

---

## Phase 2: File & Class Renaming

| Current Name            | New Name                  | New Location       |
| ----------------------- | ------------------------- | ------------------ |
| `Engine.h/cpp`          | `Application.h/cpp`       | `core/` (root)     |
| `SkyRenderer.h/cpp`     | `SkySystem.h/cpp`         | `core/atmosphere/` |
| `BlockOutline.h/cpp`    | `SelectionRenderer.h/cpp` | `core/rendering/`  |
| `TextureRegistry.h/cpp` | `TextureManager.h/cpp`    | `core/rendering/`  |
| `DisplayConfig.h/cpp`   | `VideoSettings.h/cpp`     | `core/settings/`   |
| `RenderConfig.h/cpp`    | `GraphicsSettings.h/cpp`  | `core/settings/`   |
| `Shader.h/cpp`          | (keep name)               | `core/graphics/`   |
| `Texture.h/cpp`         | (keep name)               | `core/graphics/`   |
| `TextureArray.h/cpp`    | (keep name)               | `core/graphics/`   |
| `VertexArray.h/cpp`     | (keep name)               | `core/graphics/`   |
| `VertexBuffer.h/cpp`    | (keep name)               | `core/graphics/`   |
| `IndexBuffer.h/cpp`     | (keep name)               | `core/graphics/`   |
| `Camera.h/cpp`          | (keep name)               | `core/scene/`      |
| `Frustum.h/cpp`         | (keep name)               | `core/scene/`      |
| `Ray.h`                 | (keep name)               | `core/scene/`      |
| `ShadowMap.h/cpp`       | (keep name)               | `core/rendering/`  |
| `SSAO.h/cpp`            | (keep name)               | `core/rendering/`  |
| `Window.h/cpp`          | (keep name)               | `core/window/`     |
| `GameState.h`           | (keep name)               | `core/` (root)     |
| `Logger.h`              | (keep name)               | `core/` (root)     |

---

## Phase 3: Code Naming Conventions

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
```

### 3.2 Constants Header

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

### 3.3 Shader Member Naming

```cpp
// Before (generic)                    // After (purpose-first)
std::unique_ptr<Shader> m_Shader;      std::unique_ptr<Shader> m_pLitOpaqueShader;
std::unique_ptr<Shader> m_WaterShader; std::unique_ptr<Shader> m_pWaterShader;
std::unique_ptr<Shader> m_UIShader;    std::unique_ptr<Shader> m_pUIShader;
```

> [!NOTE]
> The `p` prefix for pointers is optional. Many modern codebases skip it since `unique_ptr` is obviously a pointer. We can discuss during implementation.

---

## Implementation Steps

### Step 1: Create Directory Structure

```bash
mkdir -p core/{graphics,rendering,scene,atmosphere,window,settings}
```

### Step 2: Move Files (preserve git history)

```bash
# Graphics
git mv core/Shader.cpp core/graphics/
git mv core/Shader.h core/graphics/
git mv core/Texture.cpp core/graphics/
git mv core/Texture.h core/graphics/
git mv core/TextureArray.cpp core/graphics/
git mv core/TextureArray.h core/graphics/
git mv core/VertexArray.cpp core/graphics/
git mv core/VertexArray.h core/graphics/
git mv core/VertexBuffer.cpp core/graphics/
git mv core/VertexBuffer.h core/graphics/
git mv core/IndexBuffer.cpp core/graphics/
git mv core/IndexBuffer.h core/graphics/

# Rendering
git mv core/ShadowMap.cpp core/rendering/
git mv core/ShadowMap.h core/rendering/
git mv core/SSAO.cpp core/rendering/
git mv core/SSAO.h core/rendering/
git mv core/BlockOutline.cpp core/rendering/SelectionRenderer.cpp
git mv core/BlockOutline.h core/rendering/SelectionRenderer.h
git mv core/TextureRegistry.cpp core/rendering/TextureManager.cpp
git mv core/TextureRegistry.h core/rendering/TextureManager.h

# Scene
git mv core/Camera.cpp core/scene/
git mv core/Camera.h core/scene/
git mv core/Frustum.cpp core/scene/
git mv core/Frustum.h core/scene/
git mv core/Ray.h core/scene/

# Atmosphere
git mv core/SkyRenderer.cpp core/atmosphere/SkySystem.cpp
git mv core/SkyRenderer.h core/atmosphere/SkySystem.h

# Window
git mv core/Window.cpp core/window/
git mv core/Window.h core/window/

# Settings
git mv core/DisplayConfig.cpp core/settings/VideoSettings.cpp
git mv core/DisplayConfig.h core/settings/VideoSettings.h
git mv core/RenderConfig.cpp core/settings/GraphicsSettings.cpp
git mv core/RenderConfig.h core/settings/GraphicsSettings.h

# Rename Engine → Application (stays in root)
git mv core/Engine.cpp core/Application.cpp
git mv core/Engine.h core/Application.h
```

### Step 3: Update CMakeLists.txt

Update source file paths to reflect new structure.

### Step 4: Update All #include Paths

Search and replace all includes across the codebase.

### Step 5: Rename Classes Inside Files

Update class names to match new file names (Engine → Application, etc.).

### Step 6: Apply Boolean Prefix

Add `m_b` prefix to all boolean members.

### Step 7: Create Constants.h

Extract magic numbers into centralized constants file.

### Step 8: Build & Test

Verify everything compiles and runs correctly.

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
