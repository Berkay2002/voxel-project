# Voxel Project Brainstorming & Ideas

## Architecture Notes from `voxel.jpg`

- **Math**: GLM
- **Graphics**: OpenGL (GLAD/GLEW)
- **Window**: GLFW
- **Noise**: FastNoiseLite

## Optimization Ideas

- **Face Culling**: Don't render internal blocks.
- **Multithreading**: Chunk generation in background.
- **RLE**: for chunk storage?

## Visual Style

- Pure voxel?
- Ambient Occlusion?
- Shadows?

## Deferred Features (Future Phases)

- **Water System**: Sea level at fixed Y, water blocks, flow logic (Phase 3C or 4)
- **Cave Generation**: 3D noise carving, ore veins
- **Biomes**: Temperature/humidity maps, biome-specific block layering
- **Multiple Octaves**: Fractal noise for more natural terrain
