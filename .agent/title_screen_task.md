# Title Screen & World Loading Screen Implementation

## Tasks

### Planning
- [x] Explore existing project structure and assets
- [x] Analyze Engine loop for game state integration
- [x] Review available GUI textures (title, buttons, backgrounds)
- [x] Create implementation plan
- [x] Get user approval

### Core UI System
- [x] Create `GameState` enum (`TITLE_SCREEN`, `LOADING`, `PLAYING`)
- [x] Create `ui/UIRenderer.h/.cpp` for 2D quad rendering
- [x] Create textured UI shaders (`ui_textured.vert/frag`)
- [ ] Add font/text rendering support (deferred - using placeholders)

### Title Screen
- [x] Create `ui/TitleScreen.h/.cpp` class
- [x] Render tiled dirt background ([options_background.png](file:///c:/Users/berka/Projects/playground/voxel-project/assets/textures/gui/options_background.png))
- [x] Render game logo (VoxelCraft logo generated)
- [x] Implement "Singleplayer" button with hover state
- [x] Implement "Quit" button with hover state
- [x] Add button click handling with callbacks

### Loading Screen
- [x] Create `ui/LoadingScreen.h/.cpp` class
- [x] Render loading background
- [x] Add progress bar widget
- [x] Display loading status text (placeholder)
- [x] Integrate with ChunkManager initial load progress

### Engine Integration
- [x] Add game state management to Engine
- [x] Modify Engine::Run() loop for state-based updates
- [x] Defer world setup to Loading state
- [x] Handle state transitions (Title→Loading→Playing)
- [x] Add mouse support for menu navigation

### Verification
- [/] Build project successfully
- [ ] Test title screen rendering and button interactions
- [ ] Test loading screen progress
- [ ] Test transition to gameplay
