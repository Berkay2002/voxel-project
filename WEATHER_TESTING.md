# Weather System Testing Guide

## Overview
The weather system has been improved to use a Minecraft-style heightmap-based occlusion system. Weather particles (rain/snow) now:
1. Only appear when the player can see the sky
2. Spawn above the terrain height (not at a fixed altitude)
3. Fade out smoothly as they approach the ground
4. Are culled when they go underground or into caves

## How to Test

### 1. Enable Weather
- Press **K** to toggle weather on/off
- Weather is disabled by default

### 2. Test Scenarios

#### Scenario A: Surface Weather (Expected: Weather Visible)
1. Stand on open ground with clear sky above
2. Enable weather (K key)
3. **Expected Result**: Rain/snow particles should appear falling from above
4. Particles should fade out as they hit the ground
5. No particles should appear below your feet

#### Scenario B: Cave Weather (Expected: No Weather)
1. Enter a cave or dig underground
2. Make sure you're completely enclosed (blocks above you)
3. Enable weather (K key)
4. **Expected Result**: NO weather particles should appear
5. If you look up at the cave ceiling, no rain should be coming through

#### Scenario C: Under Overhang (Expected: No Weather)
1. Stand under a cliff overhang or tree canopy
2. Make sure there are blocks directly above you
3. Enable weather (K key)
4. **Expected Result**: NO weather should appear directly above you
5. You should see weather particles in the open area beyond the overhang

#### Scenario D: Building Interior (Expected: No Weather)
1. Build a simple house with a roof
2. Stand inside the house
3. Enable weather (K key)
4. **Expected Result**: No weather inside the house
5. Looking out windows/doors should show weather outside

#### Scenario E: Mountain Cliffs (Expected: Weather Adapts to Terrain)
1. Stand near a tall mountain or cliff face
2. Enable weather (K key)
3. **Expected Result**: 
   - Weather should spawn above the mountain peak
   - No particles should render through the cliff face
   - Particles should fade out when hitting different terrain elevations

#### Scenario F: Transition Between Areas
1. Walk from inside a cave to outside
2. Watch the weather particles as you move
3. **Expected Result**: 
   - No particles in cave
   - Particles gradually appear as you approach exit
   - Full weather once you're in the open

### 3. Visual Checks

#### Rain Behavior
- Particles should be thin vertical streaks
- Should fall quickly (25 blocks/sec by default)
- Should be semi-transparent
- Should fade at distance (32 block radius)

#### Snow Behavior  
- Particles should be small square flakes
- Should fall slowly (3 blocks/sec by default)
- Should be more opaque than rain
- Should fade at distance (32 block radius)

### 4. Performance Testing
- Weather system should have minimal performance impact
- Check FPS with weather enabled vs disabled
- Test with different chunk load distances
- Verify no stuttering when moving between different terrain heights

## Known Limitations

1. **Heightmap Resolution**: The heightmap is per-block-column (16x16 per chunk), so very thin overhangs might not be detected perfectly.

2. **Sampling Radius**: Weather uses a 3x3 block sample around the player. This means:
   - Very thin walls might occasionally let particles through
   - Large overhangs work perfectly
   - Most normal gameplay scenarios are handled correctly

3. **Particle Spawn Distance**: Particles spawn in a 32-block radius cylinder around the player. Beyond this, no weather is rendered (for performance).

## Configuration

Weather settings can be adjusted in `world/WorldConfig.h`:

```cpp
// Weather particles
constexpr float RAIN_PARTICLE_SIZE  = 0.3f;    // Width of rain streak
constexpr float RAIN_SPEED          = 25.0f;   // Fall speed (blocks/sec)
constexpr int   RAIN_DENSITY        = 1500;    // Number of particles
constexpr float SNOW_SPEED          = 3.0f;    // Slower than rain
constexpr float WEATHER_RADIUS      = 32.0f;   // Particle spawn radius
constexpr float WEATHER_SPAWN_HEIGHT = 64.0f;  // Blocks above terrain to spawn
```

## Debugging

If weather doesn't work as expected:

1. **Check Console Logs**: Look for weather-related error messages
2. **Verify Heightmap**: The heightmap should be built after terrain generation
3. **Check Shader Compilation**: Weather shader must compile successfully
4. **Verify ChunkManager**: Make sure chunks are loaded around the player

## Implementation Details

### Heightmap System
- Each chunk stores a 16x16 heightmap (one height per XZ column)
- Height = highest non-air, non-water block in the column
- Built during terrain generation and updated when blocks change
- Queried via `ChunkManager::GetHeightAt(worldX, worldZ)`

### Weather Rendering
1. CPU samples heightmap in 3x3 grid around player (9 samples)
2. Uses maximum height to determine terrain ceiling
3. Passes terrain height to vertex shader as uniform
4. Shader spawns particles above terrain height
5. Shader fades particles near ground (3-0.5 blocks)
6. Shader culls particles below terrain

### Comparison to Minecraft
This implementation mirrors Minecraft's approach:
- Heightmap-based occlusion (no weather in caves)
- Particles spawn dynamically above terrain
- Smooth fade near ground (no popping)
- Performance-friendly (pre-computed heightmap)

The main difference is that Minecraft uses a per-particle heightmap lookup, while this system uses a single heightmap value for all particles (sampled around the player). This is a reasonable trade-off for performance while maintaining visual quality.
