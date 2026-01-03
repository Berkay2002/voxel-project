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

## Phase 2: Rendering Foundations ✅

- [x] **Shader System**
  - [x] Implement `core/Shader.h/.cpp`
  - [x] Create `assets/shaders/basic.vert/frag`
  - [x] Create `assets/shaders/textured.vert/frag`
- [x] **Buffer Abstractions**
  - [x] Implement `core/VertexBuffer.h/.cpp`
  - [x] Implement `core/IndexBuffer.h/.cpp`
  - [x] Implement `core/VertexArray.h/.cpp`
- [x] **Colored Triangle** - Validated shaders + buffers
- [x] **Texture System**
  - [x] Add stb_image dependency
  - [x] Implement `core/Texture.h/.cpp`
- [x] **Textured Quad** - Validated textures + UVs
- [x] **Camera System**
  - [x] Implement `core/Camera.h/.cpp`
  - [x] WASD + mouse look controls
- [x] **Block Preview Cube** - Full 3D validation

## Phase 3: Voxel Basics (Backlog)

- [ ] Face Culling
- [ ] Multithreading
- [ ] FastNoiseLite Integration
- [ ] Infinite Terrain
