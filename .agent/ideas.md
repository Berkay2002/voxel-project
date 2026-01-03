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

### High Priority (Phase 5)

- **Multithreading**: Chunk generation in background threads (critical for performance)
- **Frustum Culling**: Don't render chunks behind camera (easy win)

### Medium Priority (Phase 6)

- **Water System**: Sea level at fixed Y (e.g., Y=40), water blocks, transparency
- **Cave Generation**: 3D Perlin noise carving tunnels, ore veins at specific depths
- **Texture Atlas**: Multiple block textures in one image, UV mapping per block type

### Lower Priority (Future)

- **Biomes**: Temperature/humidity noise maps, biome-specific terrain & blocks
- **Multiple Octaves**: Fractal Brownian Motion for more natural terrain
- **Lighting**: Sunlight propagation, ambient occlusion
- **Block Breaking/Placing**: Raycasting (DDA), player interaction

## Gameplay Ideas

- Survival mode with health/hunger?
- Creative mode with flying + infinite blocks?
- Day/night cycle with lighting changes?
- Simple mobs (slimes, zombies)?
