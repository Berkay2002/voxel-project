# Phase 2: Rendering Foundations

## Goal

Build the core rendering infrastructure: shaders, camera, textures, and prepare the foundation for voxel chunk rendering.

---

## Asset Structure

The project already has an `assets/` folder at the project root with block textures. We will add our own shaders:

```
assets/
├── shaders/           # NEW - We will create these
│   ├── basic.vert     # Simple MVP vertex shader
│   ├── basic.frag     # Solid color fragment shader
│   ├── textured.vert  # Textured mesh vertex shader
│   └── textured.frag  # Texture sampling fragment shader
└── textures/          # EXISTING - Copied textures
    ├── debug_texture.png
    └── blocks/
        ├── dirt_block.png
        ├── emerald_block.png
        ├── glowstone.png
        ├── grass_block.png
        ├── grass_block_side.png
        ├── lapis_block.png
        ├── redstone_block.png
        └── stone_block.png
```

---

## 1. Shader System

### Files

- `core/Shader.h` / `core/Shader.cpp`

### Features

- Load vertex + fragment shaders from file paths
- Compile, link, and error checking with clear logs
- Uniform setters: `SetInt`, `SetFloat`, `SetVec3`, `SetMat4`
- `Bind()` / `Unbind()` methods

### Shaders to Create

- `assets/shaders/basic.vert` - Simple MVP transform (position only)
- `assets/shaders/basic.frag` - Solid color output
- `assets/shaders/textured.vert` - MVP + pass UV coords
- `assets/shaders/textured.frag` - Sample texture at UV

---

## 2. Camera System

### Files

- `core/Camera.h` / `core/Camera.cpp`

### Features

- Perspective projection matrix
- View matrix from position + orientation
- `GetViewProjection()` combined matrix
- Keyboard movement (WASD + Space/Shift for up/down)
- Mouse look (yaw/pitch)
- Adjustable FOV, near/far planes

### Input Handling

- Integrate with `Window` class for input callbacks
- Delta time-based movement

---

## 3. Texture System

### Files

- `core/Texture.h` / `core/Texture.cpp`

### Dependencies

- **stb_image** (add via CMake - header-only library)

### Features

- Load image from file (PNG, JPG)
- Generate OpenGL texture with mipmaps
- `Bind(slot)` to texture units
- Test with existing `debug_texture.png` and block textures

### Future: Texture Atlas

- Combine block textures into single atlas for efficient voxel rendering (Phase 3)

---

## 4. Vertex Buffer Abstractions

### Files

- `core/VertexBuffer.h` / `core/VertexBuffer.cpp` (VBO)
- `core/IndexBuffer.h` / `core/IndexBuffer.cpp` (EBO)
- `core/VertexArray.h` / `core/VertexArray.cpp` (VAO)

### Features

- RAII wrappers around OpenGL buffer objects
- `VertexArray` manages attribute layout
- Support for vertex formats: position, UV, normal

---

## 5. Test Rendering Milestones

### Milestone 1: Colored Triangle

- Render a triangle with solid color shader
- Validates: Shader loading, VAO/VBO

### Milestone 2: Textured Quad

- Render a quad with `debug_texture.png`
- Validates: Texture loading, UVs, textured shader

### Milestone 3: Block Preview

- Render a textured cube using `grass_block.png` / `dirt_block.png`
- Camera can orbit around it
- Validates: Full 3D pipeline ready for voxels

---

## Implementation Order

1. **Shader System** - Required for all rendering
2. **Buffer Abstractions** (VAO/VBO/EBO) - Clean GPU data management
3. **Create basic shaders** (`assets/shaders/basic.*`)
4. **Colored Triangle** - Validate shaders + buffers
5. **Texture System** - Add stb_image, load textures
6. **Create textured shaders** (`assets/shaders/textured.*`)
7. **Textured Quad** - Combine shaders + textures
8. **Camera System** - Add movement and projection
9. **Block Preview Cube** - Full 3D validation with block textures

---

## Validation Checklist

- [ ] Shader compiles and links without errors
- [ ] Shader uniform errors are logged clearly
- [ ] Triangle renders with solid color shader
- [ ] Quad renders with `debug_texture.png`
- [ ] Camera moves with WASD + mouse
- [ ] Cube renders with block texture (e.g., grass_block.png)
- [ ] No OpenGL errors in debug output

---

## Notes

- Keep all OpenGL calls wrapped in abstractions
- Use `glGetError()` or debug callbacks for error checking
- Prepare vertex format for voxels: `{vec3 pos, vec2 uv, vec3 normal}`
- We write **simple shaders from scratch** - the existing ones were too complex
