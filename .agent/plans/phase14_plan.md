# Phase 14: Sky System (Clouds, Sun, Moon, Weather)

Complete Minecraft-style sky rendering with day/night cycle and weather effects.

## Design Principles

> [!IMPORTANT]
> **Modularity**: Each subsystem (clouds, celestials, weather) is a separate class that can be enabled/disabled independently.

> [!TIP]
> **Scalability**: Weather particles use instanced rendering. Block types and configurations are data-driven.

> [!NOTE]
> **Configurability**: All parameters in [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h) — easy to tweak without recompiling logic.

---

## Phase 14A: Cloud Layer ☁️

Flat cloud plane at fixed height with scrolling texture.

### New Files

#### [NEW] [SkyRenderer.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SkyRenderer.h)

Main sky system manager containing:

- `CloudRenderer` - Flat quad grid at Y=192
- `CelestialRenderer` - Sun/Moon billboards
- `WeatherRenderer` - Rain/snow particles (added later)

```cpp
class SkyRenderer {
public:
    void Setup();
    void Update(float deltaTime, const glm::vec3& cameraPos);
    void Render(const Camera& camera, float aspectRatio);

    void SetTimeOfDay(float time); // 0.0-1.0 (0=midnight, 0.5=noon)
    float GetTimeOfDay() const;

private:
    float m_TimeOfDay = 0.25f; // Start at dawn (6am)
    float m_DayDuration = 1200.0f; // 20 minutes per day (Minecraft default)

    // Cloud layer
    std::unique_ptr<Shader> m_CloudShader;
    GLuint m_CloudVAO, m_CloudVBO, m_CloudIBO;
    std::unique_ptr<Texture> m_CloudTexture;
    float m_CloudOffset = 0.0f;

    // Celestials
    std::unique_ptr<Shader> m_CelestialShader;
    GLuint m_SunVAO, m_MoonVAO;
    std::unique_ptr<Texture> m_SunTexture;
    std::unique_ptr<Texture> m_MoonTexture;
};
```

#### [NEW] [cloud.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/cloud.vert)

```glsl
#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 v_TexCoord;
out float v_FogFactor;

uniform mat4 u_ViewProj;
uniform vec3 u_CameraPos;
uniform float u_CloudOffset; // For eastward drift

void main() {
    vec3 worldPos = aPos;
    gl_Position = u_ViewProj * vec4(worldPos, 1.0);

    // Scroll UVs for cloud drift
    v_TexCoord = aTexCoord + vec2(u_CloudOffset, 0.0);

    // Distance fog for clouds
    float dist = distance(worldPos.xz, u_CameraPos.xz);
    v_FogFactor = clamp((dist - 80.0) / (128.0 - 80.0), 0.0, 1.0);
}
```

#### [NEW] [cloud.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/cloud.frag)

```glsl
#version 460 core
in vec2 v_TexCoord;
in float v_FogFactor;

out vec4 FragColor;

uniform sampler2D u_CloudTexture;
uniform vec3 u_FogColor;  // Sky color
uniform float u_CloudAlpha; // Overall opacity (dim at night)

void main() {
    vec4 cloudColor = texture(u_CloudTexture, v_TexCoord);

    // Minecraft clouds use alpha test, not blending (white or invisible)
    if (cloudColor.a < 0.5) discard;

    // Apply fog (blend toward sky at distance)
    cloudColor.rgb = mix(cloudColor.rgb, u_FogColor, v_FogFactor);

    // Apply time-based dimming (night = darker clouds)
    cloudColor.a *= u_CloudAlpha;

    FragColor = cloudColor;
}
```

#### [MODIFY] [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h)

Add sky configuration section:

```cpp
// =============================================================================
// SKY & WEATHER
// =============================================================================

// Clouds
constexpr float CLOUD_HEIGHT        = 192.0f;  // Y position of cloud layer
constexpr float CLOUD_SIZE          = 256.0f;  // Size of cloud plane
constexpr float CLOUD_SPEED         = 0.005f;  // Drift speed (blocks/sec relative)
constexpr float CLOUD_SCALE         = 8.0f;    // UV tiling (larger = smaller clouds)

// Day/Night cycle (Minecraft: 20 min = 24000 ticks)
constexpr float DAY_DURATION        = 1200.0f; // 20 minutes per day (Minecraft default)
constexpr float DAWN_TIME           = 0.2f;    // 4.8am
constexpr float DUSK_TIME           = 0.8f;    // 7.2pm

// Celestials
constexpr float SUN_SIZE            = 32.0f;   // Billboard size in world units
constexpr float MOON_SIZE           = 32.0f;
constexpr float SKY_RADIUS          = 200.0f;  // Distance to sun/moon from player

// Weather (toggle via K key)
constexpr float RAIN_PARTICLE_SIZE  = 0.3f;    // Width of rain streak
constexpr float RAIN_SPEED          = 20.0f;   // Fall speed (blocks/sec)
constexpr int   RAIN_DENSITY        = 2000;    // Particles per chunk
constexpr float SNOW_SPEED          = 2.0f;    // Slower than rain
constexpr bool  WEATHER_ENABLED     = false;   // Start with weather off
```

---

## Phase 14B: Sun & Moon Billboards ☀️🌙

Camera-facing sprites orbiting the player with day/night cycle.

### Texture Info (from user)

| Texture           | Dimensions | Notes                                  |
| ----------------- | ---------- | -------------------------------------- |
| [sun.png](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/textures/environment/sun.png)         | 32×32      | Single sprite                          |
| [moon_phases.png](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/textures/environment/moon_phases.png) | 128×64     | 8 phases: 16×32 each (4 cols × 2 rows) |

### CelestialRenderer Logic

```cpp
// Sun position: orbits in XY plane (0.5 = noon = directly overhead)
float sunAngle = (m_TimeOfDay - 0.25f) * 2.0f * M_PI; // Start at dawn
glm::vec3 sunDir = glm::vec3(
    cos(sunAngle),
    sin(sunAngle),
    0.0f
);
glm::vec3 sunPos = cameraPos + sunDir * SKY_RADIUS;

// Moon is opposite the sun
glm::vec3 moonPos = cameraPos - sunDir * SKY_RADIUS;

// Moon phase: changes every in-game day (8 phases)
int moonPhase = (m_DayCount % 8);
// UV offset: (phase % 4) * 0.25, (phase / 4) * 0.5
```

#### [NEW] [celestial.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/celestial.vert)

Billboard shader facing camera:

```glsl
#version 460 core
layout(location = 0) in vec3 aPos;      // Corners: (-0.5,-0.5) to (0.5,0.5)
layout(location = 1) in vec2 aTexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProj;
uniform vec3 u_WorldPos;    // Center position
uniform float u_Size;       // Billboard size
uniform vec2 u_UVOffset;    // For moon phases
uniform vec2 u_UVScale;     // For moon phases (1/4, 1/2)

uniform vec3 u_CameraRight; // Camera right vector
uniform vec3 u_CameraUp;    // Camera up vector

void main() {
    // Billboard: always face camera
    vec3 worldPos = u_WorldPos
        + u_CameraRight * aPos.x * u_Size
        + u_CameraUp * aPos.y * u_Size;

    gl_Position = u_ViewProj * vec4(worldPos, 1.0);
    v_TexCoord = aTexCoord * u_UVScale + u_UVOffset;
}
```

#### [NEW] [celestial.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/celestial.frag)

```glsl
#version 460 core
in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    vec4 color = texture(u_Texture, v_TexCoord);
    if (color.a < 0.1) discard; // Alpha test
    FragColor = color;
}
```

---

## Phase 14C: Day/Night Cycle & Lighting 🌅

Dynamic sky color and lighting based on time.

### Sky Color Interpolation

```cpp
// Dawn: orange-pink
// Day: light blue
// Dusk: orange-red
// Night: dark blue

glm::vec3 GetSkyColor(float time) {
    if (time < 0.2f) { // Night → Dawn
        return mix(NIGHT_COLOR, DAWN_COLOR, time / 0.2f);
    } else if (time < 0.3f) { // Dawn → Day
        return mix(DAWN_COLOR, DAY_COLOR, (time - 0.2f) / 0.1f);
    } else if (time < 0.7f) { // Day
        return DAY_COLOR;
    } else if (time < 0.8f) { // Day → Dusk
        return mix(DAY_COLOR, DUSK_COLOR, (time - 0.7f) / 0.1f);
    } else if (time < 0.9f) { // Dusk → Night
        return mix(DUSK_COLOR, NIGHT_COLOR, (time - 0.8f) / 0.1f);
    } else { // Night
        return NIGHT_COLOR;
    }
}
```

### Dynamic Light Direction

Sun direction uniform (`u_LightDir`) changes with time:

```cpp
// Day: sun overhead (y = 1)
// Night: sun below horizon (y = -1), use moon direction
m_Shader->SetVec3("u_LightDir", sunDir);
m_Shader->SetFloat("u_AmbientStrength", isNight ? 0.15f : 0.35f);
```

---

## Phase 14D: Weather System 🌧️❄️

Particle-based rain and snow.

### Texture Info (from user)

| Texture    | Dimensions | Notes            |
| ---------- | ---------- | ---------------- |
| [rain.png](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/textures/environment/rain.png) | 64×256     | Vertical streaks |
| [snow.png](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/textures/environment/snow.png) | 64×256     | Small flakes     |

### WeatherRenderer

Instanced rendering of falling particles:

- Spawn in cylinder around player (radius ~32 blocks)
- Rain: fast fall, long streaks, alpha blend
- Snow: slow fall, small flakes, drift

#### [NEW] [weather.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/weather.vert)

```glsl
#version 460 core
layout(location = 0) in vec3 aPos;      // Particle base position
layout(location = 1) in vec2 aTexCoord;

out vec2 v_TexCoord;
out float v_Alpha;

uniform mat4 u_ViewProj;
uniform vec3 u_CameraPos;
uniform float u_Time;
uniform float u_FallSpeed;
uniform float u_ParticleSize;

void main() {
    vec3 pos = aPos;

    // Animate fall (loop at ground level)
    pos.y -= mod(u_Time * u_FallSpeed, 128.0);
    if (pos.y < 0.0) pos.y += 128.0;

    // Billboard toward camera (only Y rotation)
    // ... billboard math

    gl_Position = u_ViewProj * vec4(pos, 1.0);
    v_TexCoord = aTexCoord;

    // Fade at distance
    float dist = distance(pos.xz, u_CameraPos.xz);
    v_Alpha = 1.0 - clamp(dist / 32.0, 0.0, 1.0);
}
```

#### [NEW] [weather.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/weather.frag)

```glsl
#version 460 core
in vec2 v_TexCoord;
in float v_Alpha;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    vec4 color = texture(u_Texture, v_TexCoord);
    color.a *= v_Alpha;
    if (color.a < 0.01) discard;
    FragColor = color;
}
```

---

## Engine Integration

### [MODIFY] [Engine.h](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.h)

```cpp
#include "world/SkyRenderer.h"

// Add member
std::unique_ptr<SkyRenderer> m_SkyRenderer;
```

### [MODIFY] [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp)

#### Render order (critical!):

```cpp
void Engine::Render() {
    // 1. Clear with dynamic sky color (from SkyRenderer)
    glm::vec3 skyColor = m_SkyRenderer->GetSkyColor();
    glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Render sky (depth write OFF, render at far plane)
    glDepthMask(GL_FALSE);
    m_SkyRenderer->Render(*m_Camera, aspectRatio);
    glDepthMask(GL_TRUE);

    // 3. Render terrain (opaque)
    m_ChunkManager->RenderAll(...);

    // 4. Render block outline
    if (m_TargetedBlock.hit) { ... }

    // 5. Render water (transparent)
    m_ChunkManager->RenderWater(...);

    // 6. Render weather particles (if active)
    if (m_SkyRenderer->IsWeatherActive()) {
        glEnable(GL_BLEND);
        m_SkyRenderer->RenderWeather(...);
        glDisable(GL_BLEND);
    }

    // 7. Render UI (crosshair)
    RenderCrosshair();
}
```

---

## File Summary

| Phase           | Files                                                         | Type         |
| --------------- | ------------------------------------------------------------- | ------------ |
| 14A (Clouds)    | `SkyRenderer.h/.cpp`, `cloud.vert/frag`, WorldConfig.h update | NEW + MODIFY |
| 14B (Sun/Moon)  | `celestial.vert/frag`, extend SkyRenderer                     | NEW + MODIFY |
| 14C (Day/Night) | Update [Engine.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/core/Engine.cpp) for dynamic lighting/sky                  | MODIFY       |
| 14D (Weather)   | `weather.vert/frag`, extend SkyRenderer                       | NEW + MODIFY |

---

## Verification Plan

### Build Verification

```bash
cd build && cmake .. && make -j$(nproc)
```

### Manual Testing (Runtime)

**Phase 14A - Clouds:**

1. Run the engine
2. Look up - clouds should be visible at Y=192
3. Wait 10-20 seconds - clouds should drift eastward
4. Walk around - clouds should extend to the fog distance

**Phase 14B - Sun/Moon:**

1. Press `T` key to fast-forward time (implement toggle)
2. Sun should rise in east, set in west
3. Moon should appear opposite sun, with correct phase sprite

**Phase 14C - Day/Night:**

1. Observe sky color change: blue → orange (dusk) → dark blue (night) → pink (dawn)
2. Terrain should get darker at night (reduced ambient)
3. Fog color should match sky color

**Phase 14D - Weather:**

1. Press `K` key to toggle rain
2. Rain particles should fall from sky
3. Particles should only appear within ~32 blocks of player
4. Walking should not show obvious particle "cylinder" edge

---

## Implementation Order

1. **Phase 14A** (Clouds) - Most visible, quick win
2. **Phase 14B** (Sun/Moon) - Adds visual polish
3. **Phase 14C** (Day/Night) - Ties it all together
4. **Phase 14D** (Weather) - Can be deferred if time is short
