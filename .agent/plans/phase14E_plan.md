# Phase 14E: Volumetric "Fancy" Clouds

Minecraft-style 3D voxel clouds with Fast/Fancy toggle.

## How Minecraft Does It

> [!NOTE]
> Minecraft extrudes a 2D cloud texture into a thin 3D volume (~4 blocks thick), then meshes only the outer faces. Animation scrolls the sampling UV coordinates, not the geometry.

**Key techniques:**
- **Occupancy mask**: Sample `clouds.png` → cell occupied if alpha > threshold
- **Chunked caching**: Build mesh in chunks, rebuild only when entering camera radius
- **Two-tone lighting**: Bottom faces darker, top/sides lighter (blocky volume feel)

---

## Proposed Changes

### [MODIFY] [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h)

```cpp
// Cloud rendering mode
enum class CloudMode { OFF, FAST, FANCY };
constexpr CloudMode CLOUD_MODE = CloudMode::FANCY;

// Volumetric cloud settings (Fancy mode)
constexpr float CLOUD_BLOCK_SIZE   = 12.0f;  // World units per cloud cell
constexpr float CLOUD_BLOCK_HEIGHT = 4.0f;   // Extrusion height (thin slab)
constexpr int   CLOUD_GRID_RADIUS  = 20;     // Cells around camera
constexpr float CLOUD_THRESHOLD    = 0.5f;   // Occupancy threshold (alpha > this = cloud)
```

---

### [MODIFY] [SkyRenderer.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SkyRenderer.h)

```cpp
// Volumetric cloud mesh
std::unique_ptr<Core::Shader> m_VolumetricCloudShader;
unsigned int m_VolumetricCloudVAO = 0;
unsigned int m_VolumetricCloudVBO = 0;
int m_VolumetricCloudVertexCount = 0;
glm::ivec2 m_LastCloudGridCenter{0, 0};  // For caching

bool SetupVolumetricClouds();
void RenderVolumetricClouds(const Core::Camera& camera, float aspectRatio);
void RebuildCloudMesh(int centerX, int centerZ);  // Rebuild when camera moves
void CleanupVolumetricClouds();

// Helper: sample clouds.png for occupancy
bool IsCloudOccupied(int gridX, int gridZ, float timeOffset);
```

---

### [MODIFY] [SkyRenderer.cpp](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SkyRenderer.cpp)

**Key changes:**

```cpp
void SkyRenderer::RebuildCloudMesh(int cx, int cz) {
    // For each cell in grid [-radius, +radius] around (cx, cz):
    //   Sample clouds.png at UV = (x * scale + drift, z * scale)
    //   If occupied (alpha > threshold):
    //     Add cube faces with neighbor culling
    //     Top face = bright, Bottom face = darker, Sides = medium
}

void SkyRenderer::RenderVolumetricClouds(const Camera& camera, float aspect) {
    // Check if camera moved to new grid cell → rebuild if needed
    int newCenterX = floor(camera.GetPosition().x / CLOUD_BLOCK_SIZE);
    int newCenterZ = floor(camera.GetPosition().z / CLOUD_BLOCK_SIZE);
    if (newCenterX != m_LastCloudGridCenter.x || 
        newCenterZ != m_LastCloudGridCenter.z) {
        RebuildCloudMesh(newCenterX, newCenterZ);
        m_LastCloudGridCenter = {newCenterX, newCenterZ};
    }
    // Render with drift offset in shader
}
```

---

### [NEW] [volumetric_cloud.vert](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/volumetric_cloud.vert)

```glsl
#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aLightLevel;  // 0.6 bottom, 0.8 side, 1.0 top

out float v_Light;
out float v_Fog;

uniform mat4 u_ViewProj;
uniform vec3 u_CameraPos;

void main() {
    gl_Position = u_ViewProj * vec4(aPos, 1.0);
    v_Light = aLightLevel;
    v_Fog = clamp(distance(aPos.xz, u_CameraPos.xz) / 200.0, 0.0, 1.0);
}
```

---

### [NEW] [volumetric_cloud.frag](file:///home/berkay-orhan/Developer/playground/voxel-project/assets/shaders/volumetric_cloud.frag)

```glsl
#version 460 core
in float v_Light;
in float v_Fog;

out vec4 FragColor;

uniform vec3 u_SkyColor;
uniform float u_Brightness;  // Day/night dimming

void main() {
    vec3 cloud = vec3(1.0) * v_Light * u_Brightness;
    cloud = mix(cloud, u_SkyColor, v_Fog);
    if (v_Fog > 0.99) discard;
    FragColor = vec4(cloud, 1.0);
}
```

---

## Implementation Order

1. Add `CloudMode` enum and config to [WorldConfig.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/WorldConfig.h)
2. Create shaders `volumetric_cloud.vert/frag`
3. Add volumetric cloud members to [SkyRenderer.h](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SkyRenderer.h)
4. Implement `RebuildCloudMesh()` with face culling + two-tone lighting
5. Implement `RenderVolumetricClouds()` with cached grid center
6. Update [Render()](file:///home/berkay-orhan/Developer/playground/voxel-project/world/SkyRenderer.cpp#106-122) to dispatch based on `CloudMode`

---

## Verification

```bash
cd /home/berkay-orhan/Developer/playground/voxel-project/build && cmake .. && make -j$(nproc)
```

**Runtime tests:**
- Clouds appear as 3D blocks with visible bottom/sides
- Bottom is darker than top (two-tone)
- Walking doesn't cause constant mesh rebuilds (only when crossing grid cells)
- Day/night cycle dims clouds correctly
