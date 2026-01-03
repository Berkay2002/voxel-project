# Voxel Engine & Game Architecture Reference

*Transcription of project architecture from `voxel.jpg`.*

## 1. Core Technologies
- **Language**: C++
- **Windowing**: GLFW ("Window")
- **Graphics**: OpenGL (using GLAD/GLEW loaders)
- **Math**: GLM

## 2. Engine Structure
- **Modular Core**: Separate engine logic from game logic.
- **Asset Manager**: Load files without hardcoding paths.
- **Key Header Files**:
    - `Camera.h`
    - `EBO.h` (Element Buffer Object)
    - `Renderer.h`
    - `Shader.h`
    - `Texture.h`
    - `VAO.h` (Vertex Array Object)
    - `VBO.h` (Vertex Buffer Object)
    - `Window.h`

## 3. Rendering Pipeline
- **Chunk System**: 
    - Dimensions: 16x16x256
    - Storage: 1D or 3D array of Block IDs.
- **Face Culling**: 
    - Logic: Never render a face touching another opaque block.
- **Raycasting**:
    - Method: DDA (Digital Differential Analyzer).

## 4. Logic & Optimization
- **Noise Generation**: FastNoiseLite (Perlin/Simplex Noise).
- **Multithreading**: Generate new chunks in the background.
- **Mesh Rebuilding Workflow**:
    1.  Update data.
    2.  Rerun face culling.
    3.  Re-generate Mesh.
    4.  Send to GPU.
