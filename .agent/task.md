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

## Phase 2: Rendering Foundations (In Progress)

- [ ] **Shader System**
  - [ ] Implement `core/Shader.h/.cpp`
  - [ ] Create `assets/shaders/basic.vert/frag`
  - [ ] Create `assets/shaders/textured.vert/frag`
- [ ] **Buffer Abstractions**
  - [ ] Implement `core/VertexBuffer.h/.cpp`
  - [ ] Implement `core/IndexBuffer.h/.cpp`
  - [ ] Implement `core/VertexArray.h/.cpp`
- [ ] **Colored Triangle** - Validate shaders + buffers
- [ ] **Texture System**
  - [ ] Add stb_image dependency
  - [ ] Implement `core/Texture.h/.cpp`
- [ ] **Textured Quad** - Validate textures + UVs
- [ ] **Camera System**
  - [ ] Implement `core/Camera.h/.cpp`
  - [ ] WASD + mouse look controls
- [ ] **Block Preview Cube** - Full 3D validation

## Phase 3: Voxel Basics (Backlog)

- [ ] Face Culling
- [ ] Multithreading
- [ ] FastNoiseLite Integration
- [ ] Infinite Terrain
