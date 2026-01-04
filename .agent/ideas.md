# Voxel Project Brainstorming & Ideas

## Architecture Notes from `voxel.jpg`

- **Math**: GLM ✅
- **Graphics**: OpenGL (GLAD) ✅
- **Window**: GLFW ✅
- **Noise**: FastNoiseLite ✅

## Implemented Features

### Core Engine (Phase 1-2) ✅

- Window management, input handling
- Shader system, buffer abstractions
- Texture system with stb_image
- Camera with WASD + mouse look

### Voxel World (Phase 3-4) ✅

- Block types with data-driven registry
- Chunk data structure (16×16×256)
- Mesh generation with face culling
- ChunkManager with dynamic loading/unloading

### Optimization (Phase 5) ✅

- **Face Culling**: Internal faces not rendered
- **Multithreading**: Async chunk generation via BS::thread_pool
- **Frustum Culling**: Chunks behind camera not rendered

### Visual (Phase 6-7, 10) ✅

- **Ambient Occlusion**: Per-vertex AO on block corners
- **Directional Lighting**: Sun with configurable direction
- **Water System**: Transparent water with separate render pass
- **Texture Array**: Multiple block textures via GL_TEXTURE_2D_ARRAY

### World Generation (Phase 8-9) ✅

- **Cave Generation**: Spaghetti caves with 3D Perlin noise
- **Biomes**: Plains and Mountains with smooth transitions
- **Rivers**: Cellular noise-based water bodies

### Block & Texture System (Phase 11) ✅

- **Data-driven blocks.json**: 18 block types
- **BlockRegistry & TextureRegistry**: Singletons for scalable management
- **Per-face textures**: Different textures for top/sides/bottom

### Player Interaction (Phase 12) ✅

- **Raycasting (DDA)**: Voxel ray traversal
- **Block Breaking**: Left-click to remove blocks
- **Block Placing**: Right-click to add blocks
- **Crosshair UI**: Minecraft-style + at screen center

## Future Ideas

### Visual Enhancements

- ~~Distance fog~~ ✅ Phase 13A
- ~~Block highlight/outline for targeted block~~ ✅ Phase 13B
- Day/night cycle with dynamic lighting
- Shadows (shadow mapping)
- Sky rendering with clouds

### World Generation

- Cheese caves (large caverns)
- Noodle caves (thin connecting tunnels)
- Trees and vegetation

### Gameplay

- Survival mode with health/hunger
- Creative mode with flying + infinite blocks
- Simple mobs (slimes, zombies)
- Inventory system / hotbar for block selection
- Save/load world to disk

### Performance

- RLE compression for chunk storage
- Level of Detail (LOD) for distant chunks
- Greedy meshing algorithm
- Occlusion culling
