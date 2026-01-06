# Core Refactoring Task

Comprehensive refactoring of `core/` directory to follow AAA game engine standards.

**Plan**: [core_refactoring_plan.md](file:///home/berkay-orhan/Developer/playground/voxel-project/.agent/plans/core_refactoring_plan.md)

## Key Decisions

- **Namespace**: Keep flat `Core::` (no nesting)
- **SkyRenderer**: Rename to `SkySystem`, defer splitting
- **Scope**: `core/` only (defer `world/` and `ui/`)
- **Booleans**: Use `m_b` prefix (Unreal-style)

---

## Phase 1: File Organization

- [ ] Create subdirectory structure
  - [ ] `core/graphics/`
  - [ ] `core/rendering/`
  - [ ] `core/scene/`
  - [ ] `core/atmosphere/`
  - [ ] `core/window/`
  - [ ] `core/settings/`
- [ ] Move files to `graphics/`: Shader, Texture, TextureArray, VertexArray, VertexBuffer, IndexBuffer
- [ ] Move files to `rendering/`: ShadowMap, SSAO, BlockOutline→SelectionRenderer, TextureRegistry→TextureManager
- [ ] Move files to `scene/`: Camera, Frustum, Ray
- [ ] Move files to `atmosphere/`: SkyRenderer→SkySystem
- [ ] Move files to `window/`: Window
- [ ] Move files to `settings/`: DisplayConfig→VideoSettings, RenderConfig→GraphicsSettings
- [ ] Rename `Engine` → `Application` (stays in core root)
- [ ] Update CMakeLists.txt with new paths

## Phase 2: Update References

- [ ] Update all `#include` paths across codebase
- [ ] Update class names inside renamed files
- [ ] Update forward declarations
- [ ] Update namespace usages (if any class name changes affect them)

## Phase 3: Code Naming Conventions

- [ ] Add `m_b` prefix to boolean members in:
  - [ ] Application.h (m_bFirstMouse, m_bCursorCaptured, m_bWorldSetupStarted)
  - [ ] SkySystem.h (m_bCloudsEnabled, m_bCelestialsEnabled, m_bWeatherEnabled)
  - [ ] Any other files with booleans
- [ ] Create `core/Constants.h` with centralized magic numbers
- [ ] Rename generic shader members to purpose-first names (optional, discuss)

## Verification

- [ ] Build passes: `cmake --build build`
- [ ] Game launches to title screen
- [ ] World renders correctly
- [ ] Day/night cycle works
- [ ] Weather toggle (K) works
- [ ] SSAO toggle (O) works
- [ ] Shadows render correctly
- [ ] Block interaction works
- [ ] Settings screen works
- [ ] Update GEMINI.md with new structure
