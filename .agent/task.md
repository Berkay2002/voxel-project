# Project Tasks

## Phase 1: Initialization & Core Engine ✅

Focus: Getting a window open, OpenGL context running, and basic engine loop.

- [x] **Project Layout & Build System**
  - [x] Create `.gitignore`
  - [x] Set up `CMakeLists.txt`
  - [x] Configure dependencies (GLFW, GLAD, GLM via FetchContent)
- [x] **Core Architecture**
  - [x] Implement `Core/Window` class (GLFW handling)
  - [x] Implement `Core/Engine` class (Main loop)
  - [x] Implement `Core/Logger` (Basic console logging)
- [x] **Graphics Verification**
  - [x] Initialize OpenGL 4.6 (GLAD)
  - [x] Clear screen with teal color
  - [ ] Render a test triangle (sanity check)

## Phase 2: Voxel Basics (Backlog)

- [ ] Shader System (`Shader.h`)
- [ ] Camera System (`Camera.h`)
- [ ] Texture System (`Texture.h`)
- [ ] Chunk Data Structure
- [ ] Basic Mesh Generation

## Phase 3: Advanced Features (Backlog)

- [ ] Face Culling
- [ ] Multithreading
- [ ] FastNoiseLite Integration
- [ ] Infinite Terrain
