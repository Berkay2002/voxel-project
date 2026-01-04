<div align="center">

![Voxel Engine Logo](logo.png)

# Voxel Engine

**A high-performance C++20 Voxel Game Engine built from scratch.**

</div>

## Overview

<img src="voxel-engine-infographic.jpg" align="right" width="400" />

This project is a custom game engine tailored for voxel-based rendering and gameplay, inspired by Minecraft. It is built using modern C++20 and focuses on performance, modularity, and clean architecture.

The engine features a modular core decoupled from game logic, a centralized configuration system, and optimized rendering techniques like aggressive face culling and multithreaded chunk generation.

### Key Features

- **Modern C++20 Architecture**: Modular design separating `core` engine components from `world` logic.
- **High-Performance Rendering**:
  - OpenGL 4.6 Core Profile.
  - Aggressive Face Culling (internal faces are never rendered).
  - Frustum Culling for optimized rendering.
  - Multithreaded Chunk Generation and Meshing.
- **Dynamic World**:
  - Infinite procedural terrain using 3D Perlin Noise (FastNoiseLite).
  - Biome System (Plains, Mountains) with smooth transitions.
  - Cave Systems: Spaghetti caves with natural entrances and water flooding.
  - Destructible Terrain: Raycast-based block breaking and placing.
- **Visuals**:
  - Dynamic Day/Night Cycle with celestial bodies.
  - Directional Lighting with Per-Vertex Ambient Occlusion (AO).
  - Transparent Water rendering with proper blending.
  - Volumetric Clouds and Distance Fog.
  - Weather System (Rain/Snow).

## Tech Stack

- **Language**: C++20
- **Build System**: CMake 3.28+
- **Windowing/Input**: GLFW 3.4
- **Graphics API**: OpenGL 4.6
- **Loaders**: GLAD
- **Math**: GLM 1.0.1
- **Noise**: FastNoiseLite 1.1.1
- **Image Loading**: stb_image
- **JSON**: nlohmann/json

## Getting Started

### Prerequisites

- C++20 compatible compiler (GCC, Clang, MSVC)
- CMake 3.28 or higher
- OpenGL 4.6 capabilities

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/voxel-project.git
cd voxel-project

# Configure the project
cmake -B build -S .

# Build the project
cmake --build build -j$(nproc)
```

### Running

```bash
./build/VoxelProject
```

## Controls

- **WASD**: Move Camera
- **Mouse**: Look Around
- **Left Click**: Break Block
- **Right Click**: Place Block
- **Shift**: Sprint / Fly Down
- **Space**: Jump / Fly Up
- **K**: Toggle Weather (Rain/Snow)
- **Esc**: Close Application

## License

This project is open source and available under the [MIT License](LICENSE).
