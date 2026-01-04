# Phase 1: Initialization & Core Engine

## Goal

Establish a robust C++20 project structure, build system, and the fundamental game loop with an OpenGL 4.5 context.

### 1. Build System Setup

- **Tool**: CMake 3.30+
- **Language Standard**: C++20
- **Dependency Management**: `FetchContent` (Automated downloading & linking)
  - **GLFW** 3.4
  - **GLAD** (OpenGL 4.6 Core Profile)
  - **GLM** 1.0.3
- **Compiler Flags**: `-Wall -Wextra -Werror` (on GCC/Clang), `/W4` (on MSVC)

### 2. Project Structure

```
.
├── .agent/          # Plans & Docs
├── core/
│   ├── Window.h/cpp      # GLFW encapsulation, Events
│   ├── Engine.h/cpp      # Main Loop (Run, Update, Render)
│   └── Logger.h/cpp      # Simple colored console logging
├── game/            # (Empty for now)
├── .gitignore       # C++, CMake, IDE ignore rules
├── main.cpp         # Entry point
└── CMakeLists.txt   # Main build script
```

### 3. Detailed Implementation Steps

1.  **Environment**: Create `.gitignore` preventing commit of build artifacts.
2.  **CMake Configuration**:
    - Setup project with C++20.
    - Use `FetchContent` to pull GLFW and GLM.
    - Integrate GLAD (either via local file or FetchContent/Generator).
3.  **Core Components**:
    - **Logger**: Implement simple macros (`LOG_INFO`, `LOG_ERROR`) for consistent output.
    - **Window**:
      - Initialize GLFW.
      - Configure GL context (Version 4.5, Core Profile).
      - Create Window.
      - Set Callbacks (Resize, Input).
    - **Engine**:
      - Initialize `Window`.
      - Initialize `GLAD` (Load GL pointers).
      - **Game Loop**: `while (!window.ShouldClose()) { Update(); Render(); }`
4.  **Verification Code**:
    - In `Render()`: `glClearColor(0.2f, 0.3f, 0.3f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);`

## Validation (Completed 2026-01-03)

- [x] CMake generation succeeds without errors.
- [x] Compiles cleanly with high warning levels.
- [x] Application opens a window with the specified title and dimensions (800x600).
- [x] Window background is the expected color (Teal).
- [x] Console shows "Engine initialized successfully!" and "OpenGL Version: 4.6.0 NVIDIA" logs.
- [x] Window responds to resize and close events (Escape key closes window).
