# Minecraft-Style Title Screen & Loading Screen

Implement a title screen and world loading screen similar to Minecraft for the voxel engine. This adds polish and a proper game flow: **Title Screen → Loading (chunk generation) → Gameplay**.

## User Review Required

> [!IMPORTANT]
> **Logo Choice**: Should I use the existing [logo.png](file:///c:/Users/berka/Projects/playground/voxel-project/logo.png) (your custom logo) or create something similar to Minecraft's title style?

> [!NOTE]
> **Scope Decision**: This plan includes basic text rendering for button labels and loading status. If you prefer just icons/graphical-only UI initially, let me know.

---

## Proposed Changes

### Game State System

Add a state machine to control game flow (menus vs gameplay).

#### [NEW] [GameState.h](file:///c:/Users/berka/Projects/playground/voxel-project/core/GameState.h)

```cpp
enum class GameState {
    TITLE_SCREEN,   // Main menu with buttons
    LOADING,        // World generation progress
    PLAYING         // Normal gameplay
};
```

---

### UI Rendering System

Create a 2D UI rendering system for menus with textured quads.

#### [NEW] [UIRenderer.h](file:///c:/Users/berka/Projects/playground/voxel-project/ui/UIRenderer.h)

Core UI rendering class:
- Render textured quads (for backgrounds, logos, buttons)
- Render colored rectangles (for progress bars)
- Support 9-slice rendering for scalable buttons
- Screen-space coordinate system (pixels)

#### [NEW] [UIRenderer.cpp](file:///c:/Users/berka/Projects/playground/voxel-project/ui/UIRenderer.cpp)

Implementation with:
- VAO/VBO for dynamic quad rendering
- Texture binding and UV management
- Alpha blending for transparency

#### [NEW] [ui_textured.vert](file:///c:/Users/berka/Projects/playground/voxel-project/assets/shaders/ui_textured.vert)

Vertex shader with position and UV coords for textured UI elements.

#### [NEW] [ui_textured.frag](file:///c:/Users/berka/Projects/playground/voxel-project/assets/shaders/ui_textured.frag)

Fragment shader sampling texture with alpha blending support.

---

### Title Screen

#### [NEW] [TitleScreen.h](file:///c:/Users/berka/Projects/playground/voxel-project/ui/TitleScreen.h)

Title screen class managing:
- Background rendering (tiled dirt texture)
- Logo display (centered at top)
- "Singleplayer" button (starts game)
- "Quit Game" button (exits app)
- Hover detection and click handling

#### [NEW] [TitleScreen.cpp](file:///c:/Users/berka/Projects/playground/voxel-project/ui/TitleScreen.cpp)

Implementation:
- Uses [options_background.png](file:///c:/Users/berka/Projects/playground/voxel-project/assets/textures/gui/options_background.png) tiled across screen
- Loads [logo.png](file:///c:/Users/berka/Projects/playground/voxel-project/logo.png) or [minecraft.png](file:///c:/Users/berka/Projects/playground/voxel-project/assets/textures/gui/title/minecraft.png) for logo
- Uses [button.png](file:///c:/Users/berka/Projects/playground/voxel-project/assets/textures/gui/sprites/widget/button.png), [button_highlighted.png](file:///c:/Users/berka/Projects/playground/voxel-project/assets/textures/gui/sprites/widget/button_highlighted.png) for buttons
- Mouse position tracking for hover states
- Click callbacks for state transitions

---

### Loading Screen

#### [NEW] [LoadingScreen.h](file:///c:/Users/berka/Projects/playground/voxel-project/ui/LoadingScreen.h)

Loading screen class managing:
- Background display
- Progress bar (0-100%)
- Status text ("Generating terrain...", "Building meshes...")

#### [NEW] [LoadingScreen.cpp](file:///c:/Users/berka/Projects/playground/voxel-project/ui/LoadingScreen.cpp)

Implementation:
- Displays progress based on chunk generation
- Returns `true` when loading complete to trigger state transition

---

### Engine Integration

#### [MODIFY] [Engine.h](file:///c:/Users/berka/Projects/playground/voxel-project/core/Engine.h)

Add:
```cpp
#include "core/GameState.h"
#include "ui/TitleScreen.h"
#include "ui/LoadingScreen.h"

// New members
GameState m_CurrentState = GameState::TITLE_SCREEN;
std::unique_ptr<UIRenderer> m_UIRenderer;
std::unique_ptr<TitleScreen> m_TitleScreen;
std::unique_ptr<LoadingScreen> m_LoadingScreen;

// New methods
void UpdateTitleScreen();
void UpdateLoadingScreen(float deltaTime);
void RenderUI();
void TransitionToState(GameState newState);
```

#### [MODIFY] [Engine.cpp](file:///c:/Users/berka/Projects/playground/voxel-project/core/Engine.cpp)

Modify [Run()](file:///c:/Users/berka/Projects/playground/voxel-project/core/Engine.cpp#370-387) loop:
```cpp
void Engine::Run() {
    while (!m_Window->ShouldClose()) {
        // ... delta time calc ...
        
        switch (m_CurrentState) {
            case GameState::TITLE_SCREEN:
                UpdateTitleScreen();
                RenderTitleScreen();
                break;
            case GameState::LOADING:
                UpdateLoadingScreen(deltaTime);
                RenderLoadingScreen();
                break;
            case GameState::PLAYING:
                Update(deltaTime);
                Render();
                break;
        }
        
        m_Window->SwapBuffers();
    }
}
```

Defer [SetupWorld()](file:///c:/Users/berka/Projects/playground/voxel-project/core/Engine.cpp#96-254) to when "Singleplayer" is clicked.

---

### CMake Update

#### [MODIFY] [CMakeLists.txt](file:///c:/Users/berka/Projects/playground/voxel-project/CMakeLists.txt)

Add new source files:
```cmake
set(UI_SOURCES
    ui/UIRenderer.cpp
    ui/TitleScreen.cpp
    ui/LoadingScreen.cpp
)
```

---

## Verification Plan

### Build Verification

```bash
cd c:\Users\berka\Projects\playground\voxel-project\build
cmake --build . --config Release
```

Success criteria: Build completes without errors.

### Manual Testing

1. **Launch Game**: Run the executable
   - Title screen should appear with dirt background, logo, and buttons
   
2. **Button Hover**: Move mouse over buttons
   - Buttons should highlight when hovered
   
3. **Quit Button**: Click "Quit Game"
   - Window should close
   
4. **Singleplayer Button**: Click "Singleplayer"
   - Loading screen should appear with progress bar
   - Progress should increase as chunks generate
   - After loading, gameplay should start
   
5. **Gameplay**: Verify normal gameplay works
   - WASD movement, mouse look, block interactions

---

## File Summary

| Action | File |
|--------|------|
| NEW | `core/GameState.h` |
| NEW | `ui/UIRenderer.h` |
| NEW | `ui/UIRenderer.cpp` |
| NEW | `ui/TitleScreen.h` |
| NEW | `ui/TitleScreen.cpp` |
| NEW | `ui/LoadingScreen.h` |
| NEW | `ui/LoadingScreen.cpp` |
| NEW | `assets/shaders/ui_textured.vert` |
| NEW | `assets/shaders/ui_textured.frag` |
| MODIFY | [core/Engine.h](file:///c:/Users/berka/Projects/playground/voxel-project/core/Engine.h) |
| MODIFY | [core/Engine.cpp](file:///c:/Users/berka/Projects/playground/voxel-project/core/Engine.cpp) |
| MODIFY | [CMakeLists.txt](file:///c:/Users/berka/Projects/playground/voxel-project/CMakeLists.txt) |
