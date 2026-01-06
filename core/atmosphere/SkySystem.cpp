#include "SkySystem.h"
#include "world/WorldConfig.h"
#include "core/scene/Camera.h"
#include "core/Logger.h"
#include "core/graphics/Shader.h"
#include "core/graphics/Texture.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Core {

// =============================================================================
// SKY COLORS FOR DAY/NIGHT CYCLE
// =============================================================================

namespace SkyColors {
    // Minecraft-inspired sky colors
    constexpr glm::vec3 NIGHT  = glm::vec3(0.01f, 0.01f, 0.05f);   // Dark blue-black
    constexpr glm::vec3 DAWN   = glm::vec3(0.95f, 0.55f, 0.35f);   // Orange-pink
    constexpr glm::vec3 DAY    = glm::vec3(0.5f, 0.7f, 1.0f);      // Light blue
    constexpr glm::vec3 DUSK   = glm::vec3(0.9f, 0.4f, 0.3f);      // Orange-red
}

// =============================================================================
// CONSTRUCTOR / DESTRUCTOR
// =============================================================================

SkyRenderer::SkyRenderer() = default;

SkyRenderer::~SkyRenderer() {
    CleanupClouds();
    CleanupVolumetricClouds();
    CleanupCelestials();
    CleanupWeather();
}

// =============================================================================
// SETUP
// =============================================================================

bool SkyRenderer::Setup() {
    LOG_INFO("Setting up SkyRenderer...");
    
    bool success = true;
    
    if (!SetupClouds()) {
        LOG_ERROR("Failed to setup clouds");
        success = false;
    }
    
    if (!SetupVolumetricClouds()) {
        LOG_ERROR("Failed to setup volumetric clouds");
        success = false;
    }
    
    if (!SetupCelestials()) {
        LOG_ERROR("Failed to setup celestials");
        success = false;
    }
    
    if (!SetupWeather()) {
        LOG_ERROR("Failed to setup weather");
        success = false;
    }
    
    if (success) {
        LOG_INFO("SkyRenderer setup complete");
    }
    
    return success;
}

// =============================================================================
// UPDATE
// =============================================================================

void SkyRenderer::Update(float deltaTime, const glm::vec3& cameraPos) {
    // Update time of day (Minecraft: 20 min = full day)
    float dayProgress = deltaTime / Voxel::Config::DAY_DURATION;
    m_TimeOfDay += dayProgress;
    
    // Wrap to next day
    if (m_TimeOfDay >= 1.0f) {
        m_TimeOfDay -= 1.0f;
        m_DayCount++;
    }
    
    // Update cloud drift (eastward = +X = +U in UV space)
    m_CloudOffset += Voxel::Config::CLOUD_SPEED * deltaTime;
    if (m_CloudOffset > 100.0f) {
        m_CloudOffset -= 100.0f;  // Prevent float precision issues
    }
    
    // Store camera position for weather particles
    m_LastCameraPos = cameraPos;
}

// =============================================================================
// RENDER
// =============================================================================

void SkyRenderer::Render(const Camera& camera, float aspectRatio) {
    // Get view-projection matrix
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProj = proj * view;
    (void)viewProj;  // Reserved for future use
    
    // Render celestials first (behind clouds)
    if (m_CelestialsEnabled) {
        RenderCelestials(camera, aspectRatio);
    }
    
    // Render clouds based on CloudMode setting
    if (m_CloudsEnabled) {
        switch (Voxel::Config::CLOUD_MODE) {
            case Voxel::Config::CloudMode::OFF:
                // Clouds disabled
                break;
            case Voxel::Config::CloudMode::FAST:
                // 2D flat cloud plane
                RenderClouds(camera, aspectRatio);
                break;
            case Voxel::Config::CloudMode::FANCY:
                // 3D volumetric cloud voxels
                RenderVolumetricClouds(camera, aspectRatio);
                break;
        }
    }
}

void SkyRenderer::RenderWeather(const Camera& camera, float aspectRatio) {
    if (!m_WeatherEnabled || !m_WeatherShader || !m_WeatherShader->IsValid()) {
        return;
    }
    
    // Select texture based on weather type
    Texture* texture = nullptr;
    float fallSpeed = Voxel::Config::RAIN_SPEED;
    float particleHeight = 1.0f;
    float particleWidth = Voxel::Config::RAIN_PARTICLE_SIZE;
    
    switch (m_WeatherType) {
        case WeatherType::Rain:
            texture = m_RainTexture.get();
            fallSpeed = Voxel::Config::RAIN_SPEED;
            particleHeight = 1.5f;  // Long streaks
            break;
        case WeatherType::Snow:
            texture = m_SnowTexture.get();
            fallSpeed = Voxel::Config::SNOW_SPEED;
            particleHeight = 0.5f;  // Short flakes
            particleWidth = 0.2f;
            break;
        default:
            return;
    }
    
    if (!texture || !texture->IsValid()) {
        return;
    }
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProj = proj * view;
    
    glm::vec3 cameraRight = glm::normalize(glm::vec3(view[0][0], view[1][0], view[2][0]));
    
    m_WeatherShader->Bind();
    m_WeatherShader->SetMat4("u_ViewProj", viewProj);
    m_WeatherShader->SetVec3("u_CameraPos", camera.GetPosition());
    m_WeatherShader->SetVec3("u_CameraRight", cameraRight);
    m_WeatherShader->SetFloat("u_Time", static_cast<float>(glfwGetTime()));
    m_WeatherShader->SetFloat("u_FallSpeed", fallSpeed);
    m_WeatherShader->SetFloat("u_ParticleHeight", particleHeight);
    m_WeatherShader->SetFloat("u_ParticleWidth", particleWidth);
    
    texture->Bind(0);
    m_WeatherShader->SetInt("u_Texture", 0);
    
    glBindVertexArray(m_WeatherVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_WeatherParticleCount * 6);  // 6 vertices per particle (2 triangles)
    glBindVertexArray(0);
    
    texture->Unbind();
    m_WeatherShader->Unbind();
}

// =============================================================================
// TIME OF DAY / SKY COLOR
// =============================================================================

void SkyRenderer::SetTimeOfDay(float time) {
    // Properly wrap time and update day count (for moon phases)
    while (time >= 1.0f) {
        time -= 1.0f;
        m_DayCount++;
    }
    while (time < 0.0f) {
        time += 1.0f;
        m_DayCount--;
    }
    m_TimeOfDay = time;
}

glm::vec3 SkyRenderer::GetSkyColor() const {
    float t = m_TimeOfDay;
    
    // Time periods:
    // 0.0-0.2: Night
    // 0.2-0.3: Dawn transition
    // 0.3-0.7: Day
    // 0.7-0.8: Dusk transition
    // 0.8-1.0: Night
    
    if (t < 0.2f) {
        // Night
        return SkyColors::NIGHT;
    } else if (t < 0.25f) {
        // Night → Dawn
        float blend = (t - 0.2f) / 0.05f;
        return glm::mix(SkyColors::NIGHT, SkyColors::DAWN, blend);
    } else if (t < 0.3f) {
        // Dawn → Day
        float blend = (t - 0.25f) / 0.05f;
        return glm::mix(SkyColors::DAWN, SkyColors::DAY, blend);
    } else if (t < 0.7f) {
        // Day
        return SkyColors::DAY;
    } else if (t < 0.75f) {
        // Day → Dusk
        float blend = (t - 0.7f) / 0.05f;
        return glm::mix(SkyColors::DAY, SkyColors::DUSK, blend);
    } else if (t < 0.8f) {
        // Dusk → Night
        float blend = (t - 0.75f) / 0.05f;
        return glm::mix(SkyColors::DUSK, SkyColors::NIGHT, blend);
    } else {
        // Night
        return SkyColors::NIGHT;
    }
}

glm::vec3 SkyRenderer::GetSunDirection() const {
    // Sun orbits in the XY plane
    // 0.0 = midnight (below horizon), 0.25 = dawn, 0.5 = noon (overhead), 0.75 = dusk
    float angle = (m_TimeOfDay - 0.25f) * 2.0f * static_cast<float>(M_PI);
    
    glm::vec3 sunDir = glm::normalize(glm::vec3(
        std::cos(angle),   // X: east-west
        std::sin(angle),   // Y: up-down
        0.0f               // Z: no tilt (pure vertical orbit)
    ));
    
    // Smooth transition near horizon to avoid abrupt lighting changes
    // When sun is near horizon (Y close to 0), blend towards a "null" light direction
    // This creates a gradual dawn/dusk transition instead of a hard cutoff
    const float horizonThreshold = 0.15f;  // Start fading when sun is this close to horizon
    
    if (sunDir.y < 0.0f) {
        // Sun below horizon - no directional lighting
        return glm::vec3(0.0f, -1.0f, 0.0f);
    } else if (sunDir.y < horizonThreshold) {
        // Sun near horizon - blend between null light and actual sun direction
        float t = sunDir.y / horizonThreshold;  // 0 at horizon, 1 at threshold
        glm::vec3 nullLight = glm::vec3(0.0f, -1.0f, 0.0f);
        return glm::normalize(glm::mix(nullLight, sunDir, t));
    }
    
    return sunDir;
}

float SkyRenderer::GetAmbientStrength() const {
    float t = m_TimeOfDay;
    
    // Lower ambient at night
    if (t < 0.2f || t > 0.8f) {
        return 0.15f;  // Night
    } else if (t < 0.3f) {
        // Dawn transition
        float blend = (t - 0.2f) / 0.1f;
        return glm::mix(0.15f, 0.35f, blend);
    } else if (t > 0.7f) {
        // Dusk transition
        float blend = (t - 0.7f) / 0.1f;
        return glm::mix(0.35f, 0.15f, blend);
    }
    
    return 0.35f;  // Day
}

// =============================================================================
// CLOUD LAYER IMPLEMENTATION
// =============================================================================

bool SkyRenderer::SetupClouds() {
    // Load cloud shader
    m_CloudShader = std::make_unique<Shader>(
        "assets/shaders/cloud.vert",
        "assets/shaders/cloud.frag"
    );
    
    if (!m_CloudShader->IsValid()) {
        LOG_ERROR("Failed to load cloud shader");
        return false;
    }
    
    // Load cloud texture
    m_CloudTexture = std::make_unique<Texture>(
        "assets/textures/environment/clouds.png"
    );
    
    if (!m_CloudTexture->IsValid()) {
        LOG_ERROR("Failed to load clouds.png");
        return false;
    }
    
    // Create cloud plane mesh (large quad centered at origin, at cloud height)
    // Grid of tiles for better UV precision
    const float cloudSize = Voxel::Config::CLOUD_SIZE;
    const float cloudHeight = Voxel::Config::CLOUD_HEIGHT;
    const int gridSize = 8;  // 8x8 grid of quads
    const float tileSize = cloudSize / gridSize;
    const float uvScale = Voxel::Config::CLOUD_SCALE;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            float x0 = (x - gridSize / 2.0f) * tileSize;
            float x1 = x0 + tileSize;
            float z0 = (z - gridSize / 2.0f) * tileSize;
            float z1 = z0 + tileSize;
            
            float u0 = x * uvScale / gridSize;
            float u1 = (x + 1) * uvScale / gridSize;
            float v0 = z * uvScale / gridSize;
            float v1 = (z + 1) * uvScale / gridSize;
            
            unsigned int baseIdx = static_cast<unsigned int>(vertices.size() / 5);
            
            // 4 vertices per quad: pos(3) + uv(2)
            // Bottom-left
            vertices.insert(vertices.end(), {x0, cloudHeight, z0, u0, v0});
            // Bottom-right
            vertices.insert(vertices.end(), {x1, cloudHeight, z0, u1, v0});
            // Top-right
            vertices.insert(vertices.end(), {x1, cloudHeight, z1, u1, v1});
            // Top-left
            vertices.insert(vertices.end(), {x0, cloudHeight, z1, u0, v1});
            
            // 2 triangles per quad
            indices.insert(indices.end(), {
                baseIdx, baseIdx + 1, baseIdx + 2,
                baseIdx, baseIdx + 2, baseIdx + 3
            });
        }
    }
    
    m_CloudIndexCount = static_cast<int>(indices.size());
    
    // Create VAO/VBO/IBO
    glGenVertexArrays(1, &m_CloudVAO);
    glGenBuffers(1, &m_CloudVBO);
    glGenBuffers(1, &m_CloudIBO);
    
    glBindVertexArray(m_CloudVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_CloudVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CloudIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    LOG_INFO("Cloud layer initialized with " + std::to_string(gridSize * gridSize) + " tiles");
    return true;
}

void SkyRenderer::RenderClouds(const Camera& camera, float aspectRatio) {
    if (!m_CloudShader || !m_CloudShader->IsValid() || !m_CloudTexture || !m_CloudTexture->IsValid()) {
        return;
    }
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProj = proj * view;
    
    // Calculate cloud brightness based on time of day
    float brightness = 1.0f;
    float t = m_TimeOfDay;
    if (t < 0.2f || t > 0.8f) {
        brightness = 0.3f;  // Night
    } else if (t < 0.3f) {
        brightness = glm::mix(0.3f, 1.0f, (t - 0.2f) / 0.1f);  // Dawn
    } else if (t > 0.7f) {
        brightness = glm::mix(1.0f, 0.3f, (t - 0.7f) / 0.1f);  // Dusk
    }
    
    m_CloudShader->Bind();
    m_CloudShader->SetMat4("u_ViewProj", viewProj);
    m_CloudShader->SetVec3("u_CameraPos", camera.GetPosition());
    m_CloudShader->SetFloat("u_CloudOffset", m_CloudOffset);
    m_CloudShader->SetVec3("u_SkyColor", GetSkyColor());
    m_CloudShader->SetFloat("u_CloudBrightness", brightness);
    
    m_CloudTexture->Bind(0);
    m_CloudShader->SetInt("u_CloudTexture", 0);
    
    // Disable backface culling for clouds (visible from below)
    glDisable(GL_CULL_FACE);
    
    glBindVertexArray(m_CloudVAO);
    glDrawElements(GL_TRIANGLES, m_CloudIndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    
    glEnable(GL_CULL_FACE);
    
    m_CloudTexture->Unbind();
    m_CloudShader->Unbind();
}

void SkyRenderer::CleanupClouds() {
    if (m_CloudVAO != 0) {
        glDeleteVertexArrays(1, &m_CloudVAO);
        m_CloudVAO = 0;
    }
    if (m_CloudVBO != 0) {
        glDeleteBuffers(1, &m_CloudVBO);
        m_CloudVBO = 0;
    }
    if (m_CloudIBO != 0) {
        glDeleteBuffers(1, &m_CloudIBO);
        m_CloudIBO = 0;
    }
}

// =============================================================================
// VOLUMETRIC CLOUDS IMPLEMENTATION (Fancy 3D mode)
// =============================================================================

bool SkyRenderer::SetupVolumetricClouds() {
    // Load volumetric cloud shader
    m_VolumetricCloudShader = std::make_unique<Shader>(
        "assets/shaders/volumetric_cloud.vert",
        "assets/shaders/volumetric_cloud.frag"
    );
    
    if (!m_VolumetricCloudShader->IsValid()) {
        LOG_ERROR("Failed to load volumetric cloud shader");
        return false;
    }
    
    // Initialize FastNoiseLite for organic cloud shapes
    m_CloudNoise = std::make_unique<FastNoiseLite>(Voxel::Config::TERRAIN_SEED + 999);
    m_CloudNoise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_CloudNoise->SetFrequency(Voxel::Config::CLOUD_NOISE_SCALE);
    // Use FBm (Fractal Brownian Motion) for multi-octave detail
    m_CloudNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
    m_CloudNoise->SetFractalOctaves(3);        // 3 layers of detail
    m_CloudNoise->SetFractalLacunarity(2.0f);  // Frequency multiplier per octave
    m_CloudNoise->SetFractalGain(0.5f);        // Amplitude reduction per octave
    
    // Create initial empty VAO/VBO - mesh will be built on first render
    glGenVertexArrays(1, &m_VolumetricCloudVAO);
    glGenBuffers(1, &m_VolumetricCloudVBO);
    
    // Reset grid center to force initial mesh build
    m_LastCloudGridCenter = glm::ivec2(INT_MAX, INT_MAX);
    
    LOG_INFO("Volumetric cloud system initialized with FastNoiseLite");
    return true;
}

bool SkyRenderer::IsCloudOccupied(int gridX, int gridZ) const {
    if (!m_CloudNoise) return false;
    
    // Apply drift offset for animation (cloud movement)
    float x = static_cast<float>(gridX) + m_CloudOffset * 8.0f;
    float z = static_cast<float>(gridZ);
    
    // Sample FastNoiseLite - returns values in [-1, 1] range
    float noiseValue = m_CloudNoise->GetNoise(x, z);
    
    // Normalize to [0, 1] range for threshold comparison
    float normalized = (noiseValue + 1.0f) * 0.5f;
    
    return normalized > Voxel::Config::CLOUD_THRESHOLD;
}

void SkyRenderer::RebuildCloudMesh(int centerX, int centerZ) {
    // Vertex structure: pos(3) + normal(3) + lightLevel(1)
    std::vector<float> vertices;
    vertices.reserve(Voxel::Config::CLOUD_GRID_RADIUS * Voxel::Config::CLOUD_GRID_RADIUS * 36 * 7);  // Rough estimate
    
    const float blockSize = Voxel::Config::CLOUD_BLOCK_SIZE;
    const float cloudY = Voxel::Config::CLOUD_HEIGHT;
    const float cloudHeight = Voxel::Config::CLOUD_BLOCK_HEIGHT;
    const int radius = Voxel::Config::CLOUD_GRID_RADIUS;
    
    // Helper to add a face with pre-baked lighting
    auto addFace = [&](float x, float y, float z, float size, float height,
                       float nx, float ny, float nz, float light) {
        // Determine face orientation and generate quad vertices
        if (ny > 0.5f) {
            // Top face (+Y)
            float y1 = y + height;
            vertices.insert(vertices.end(), {x, y1, z,             nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y1, z,      nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y1, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y1, z,             nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y1, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y1, z + size,      nx, ny, nz, light});
        } else if (ny < -0.5f) {
            // Bottom face (-Y)
            vertices.insert(vertices.end(), {x, y, z + size,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y, z + size,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y, z,              nx, ny, nz, light});
        } else if (nz > 0.5f) {
            // Front face (+Z)
            vertices.insert(vertices.end(), {x, y, z + size,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y + height, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y, z + size,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y + height, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y + height, z + size, nx, ny, nz, light});
        } else if (nz < -0.5f) {
            // Back face (-Z)
            vertices.insert(vertices.end(), {x + size, y, z,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y, z,              nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y + height, z,     nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y + height, z,     nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y + height, z, nx, ny, nz, light});
        } else if (nx > 0.5f) {
            // Right face (+X)
            vertices.insert(vertices.end(), {x + size, y, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y + height, z, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y + height, z, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x + size, y + height, z + size, nx, ny, nz, light});
        } else if (nx < -0.5f) {
            // Left face (-X)
            vertices.insert(vertices.end(), {x, y, z,              nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y, z + size,       nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y + height, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y, z,              nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y + height, z + size, nx, ny, nz, light});
            vertices.insert(vertices.end(), {x, y + height, z,     nx, ny, nz, light});
        }
    };
    
    // Iterate over cloud grid
    for (int gz = -radius; gz <= radius; gz++) {
        for (int gx = -radius; gx <= radius; gx++) {
            int worldGX = centerX + gx;
            int worldGZ = centerZ + gz;
            
            if (!IsCloudOccupied(worldGX, worldGZ)) continue;
            
            // Calculate world position of this cloud block
            float x = static_cast<float>(worldGX) * blockSize;
            float z = static_cast<float>(worldGZ) * blockSize;
            
            // Check neighbors for face culling
            bool hasTop = true;     // Always render top (no clouds above)
            bool hasBottom = true;  // Always render bottom (no clouds below)
            bool hasFront = !IsCloudOccupied(worldGX, worldGZ + 1);
            bool hasBack = !IsCloudOccupied(worldGX, worldGZ - 1);
            bool hasRight = !IsCloudOccupied(worldGX + 1, worldGZ);
            bool hasLeft = !IsCloudOccupied(worldGX - 1, worldGZ);
            
            // Add visible faces with two-tone lighting
            if (hasTop)    addFace(x, cloudY, z, blockSize, cloudHeight, 0, 1, 0, Voxel::Config::CLOUD_LIGHT_TOP);
            if (hasBottom) addFace(x, cloudY, z, blockSize, cloudHeight, 0, -1, 0, Voxel::Config::CLOUD_LIGHT_BOTTOM);
            if (hasFront)  addFace(x, cloudY, z, blockSize, cloudHeight, 0, 0, 1, Voxel::Config::CLOUD_LIGHT_SIDE);
            if (hasBack)   addFace(x, cloudY, z, blockSize, cloudHeight, 0, 0, -1, Voxel::Config::CLOUD_LIGHT_SIDE);
            if (hasRight)  addFace(x, cloudY, z, blockSize, cloudHeight, 1, 0, 0, Voxel::Config::CLOUD_LIGHT_SIDE);
            if (hasLeft)   addFace(x, cloudY, z, blockSize, cloudHeight, -1, 0, 0, Voxel::Config::CLOUD_LIGHT_SIDE);
        }
    }
    
    m_VolumetricCloudVertexCount = static_cast<int>(vertices.size() / 7);
    
    // Upload to GPU
    glBindVertexArray(m_VolumetricCloudVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VolumetricCloudVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Light level (location 2)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void SkyRenderer::RenderVolumetricClouds(const Camera& camera, float aspectRatio) {
    if (!m_VolumetricCloudShader || !m_VolumetricCloudShader->IsValid()) {
        return;
    }
    
    glm::vec3 camPos = camera.GetPosition();
    
    // Check if camera moved to a new grid cell - rebuild mesh if needed
    int newCenterX = static_cast<int>(std::floor(camPos.x / Voxel::Config::CLOUD_BLOCK_SIZE));
    int newCenterZ = static_cast<int>(std::floor(camPos.z / Voxel::Config::CLOUD_BLOCK_SIZE));
    
    if (newCenterX != m_LastCloudGridCenter.x || newCenterZ != m_LastCloudGridCenter.y) {
        RebuildCloudMesh(newCenterX, newCenterZ);
        m_LastCloudGridCenter = glm::ivec2(newCenterX, newCenterZ);
    }
    
    if (m_VolumetricCloudVertexCount == 0) return;
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProj = proj * view;
    
    // Calculate cloud brightness based on time of day
    float brightness = 1.0f;
    float t = m_TimeOfDay;
    if (t < 0.2f || t > 0.8f) {
        brightness = 0.3f;
    } else if (t < 0.3f) {
        brightness = glm::mix(0.3f, 1.0f, (t - 0.2f) / 0.1f);
    } else if (t > 0.7f) {
        brightness = glm::mix(1.0f, 0.3f, (t - 0.7f) / 0.1f);
    }
    
    m_VolumetricCloudShader->Bind();
    m_VolumetricCloudShader->SetMat4("u_ViewProj", viewProj);
    m_VolumetricCloudShader->SetVec3("u_CameraPos", camPos);
    m_VolumetricCloudShader->SetVec3("u_SkyColor", GetSkyColor());
    m_VolumetricCloudShader->SetVec3("u_SunDirection", GetSunDirection());
    m_VolumetricCloudShader->SetFloat("u_Brightness", brightness);
    m_VolumetricCloudShader->SetFloat("u_FogStart", 80.0f);
    m_VolumetricCloudShader->SetFloat("u_FogEnd", Voxel::Config::CLOUD_GRID_RADIUS * Voxel::Config::CLOUD_BLOCK_SIZE * 0.9f);
    
    // Disable backface culling so clouds visible from inside
    glDisable(GL_CULL_FACE);
    
    glBindVertexArray(m_VolumetricCloudVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VolumetricCloudVertexCount);
    glBindVertexArray(0);
    
    glEnable(GL_CULL_FACE);
    
    m_VolumetricCloudShader->Unbind();
}

void SkyRenderer::CleanupVolumetricClouds() {
    if (m_VolumetricCloudVAO != 0) {
        glDeleteVertexArrays(1, &m_VolumetricCloudVAO);
        m_VolumetricCloudVAO = 0;
    }
    if (m_VolumetricCloudVBO != 0) {
        glDeleteBuffers(1, &m_VolumetricCloudVBO);
        m_VolumetricCloudVBO = 0;
    }
}

// =============================================================================
// CELESTIALS IMPLEMENTATION (Sun/Moon)
// =============================================================================

bool SkyRenderer::SetupCelestials() {
    // Load celestial shader
    m_CelestialShader = std::make_unique<Shader>(
        "assets/shaders/celestial.vert",
        "assets/shaders/celestial.frag"
    );
    
    if (!m_CelestialShader->IsValid()) {
        LOG_ERROR("Failed to load celestial shader");
        return false;
    }
    
    // Load sun texture
    m_SunTexture = std::make_unique<Texture>(
        "assets/textures/environment/sun.png"
    );
    
    if (!m_SunTexture->IsValid()) {
        LOG_ERROR("Failed to load sun.png");
        return false;
    }
    
    // Load moon texture
    m_MoonTexture = std::make_unique<Texture>(
        "assets/textures/environment/moon_phases.png"
    );
    
    if (!m_MoonTexture->IsValid()) {
        LOG_ERROR("Failed to load moon_phases.png");
        return false;
    }
    
    // Create billboard quad (centered unit square, -0.5 to 0.5)
    float billboardVertices[] = {
        // Position (x, y, z)    TexCoord (u, v)
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f,  // Top-right
         
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,  // Bottom-left
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f, 0.0f,     0.0f, 1.0f   // Top-left
    };
    
    glGenVertexArrays(1, &m_BillboardVAO);
    glGenBuffers(1, &m_BillboardVBO);
    
    glBindVertexArray(m_BillboardVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_BillboardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVertices), billboardVertices, GL_STATIC_DRAW);
    
    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    LOG_INFO("Celestials (sun/moon) initialized");
    return true;
}

void SkyRenderer::RenderCelestials(const Camera& camera, float aspectRatio) {
    if (!m_CelestialShader || !m_CelestialShader->IsValid()) {
        return;
    }
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProj = proj * view;
    
    // Extract camera vectors for billboarding
    glm::vec3 cameraRight = glm::normalize(glm::vec3(view[0][0], view[1][0], view[2][0]));
    glm::vec3 cameraUp = glm::normalize(glm::vec3(view[0][1], view[1][1], view[2][1]));
    
    glm::vec3 cameraPos = camera.GetPosition();
    
    // Calculate sun/moon positions
    float sunAngle = (m_TimeOfDay - 0.25f) * 2.0f * static_cast<float>(M_PI);
    glm::vec3 sunDir = glm::vec3(std::cos(sunAngle), std::sin(sunAngle), 0.0f);
    glm::vec3 sunPos = cameraPos + sunDir * Voxel::Config::SKY_RADIUS;
    glm::vec3 moonPos = cameraPos - sunDir * Voxel::Config::SKY_RADIUS;  // Opposite sun
    
    m_CelestialShader->Bind();
    m_CelestialShader->SetMat4("u_ViewProj", viewProj);
    m_CelestialShader->SetVec3("u_CameraRight", cameraRight);
    m_CelestialShader->SetVec3("u_CameraUp", cameraUp);
    
    glBindVertexArray(m_BillboardVAO);
    
    // Render sun (only if above horizon: Y > 0 relative to camera)
    if (sunDir.y > -0.2f && m_SunTexture && m_SunTexture->IsValid()) {
        m_CelestialShader->SetVec3("u_WorldPos", sunPos);
        m_CelestialShader->SetFloat("u_Size", Voxel::Config::SUN_SIZE);
        m_CelestialShader->SetVec2("u_UVOffset", glm::vec2(0.0f, 0.0f));
        m_CelestialShader->SetVec2("u_UVScale", glm::vec2(1.0f, 1.0f));
        
        m_SunTexture->Bind(0);
        m_CelestialShader->SetInt("u_Texture", 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m_SunTexture->Unbind();
    }
    
    // Render moon (only if above horizon: opposite sun)
    if (sunDir.y < 0.2f && m_MoonTexture && m_MoonTexture->IsValid()) {
        // Calculate moon phase UV offset
        // moon_phases.png is 128x64, 8 phases: 4 columns x 2 rows, each 32x32
        int phase = GetMoonPhase();
        float uOffset = (phase % 4) * 0.25f;
        float vOffset = (phase / 4) * 0.5f;
        
        m_CelestialShader->SetVec3("u_WorldPos", moonPos);
        m_CelestialShader->SetFloat("u_Size", Voxel::Config::MOON_SIZE);
        m_CelestialShader->SetVec2("u_UVOffset", glm::vec2(uOffset, vOffset));
        m_CelestialShader->SetVec2("u_UVScale", glm::vec2(0.25f, 0.5f));
        
        m_MoonTexture->Bind(0);
        m_CelestialShader->SetInt("u_Texture", 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m_MoonTexture->Unbind();
    }
    
    glBindVertexArray(0);
    m_CelestialShader->Unbind();
}

void SkyRenderer::CleanupCelestials() {
    if (m_BillboardVAO != 0) {
        glDeleteVertexArrays(1, &m_BillboardVAO);
        m_BillboardVAO = 0;
    }
    if (m_BillboardVBO != 0) {
        glDeleteBuffers(1, &m_BillboardVBO);
        m_BillboardVBO = 0;
    }
}

// =============================================================================
// WEATHER SYSTEM IMPLEMENTATION
// =============================================================================

bool SkyRenderer::SetupWeather() {
    // Load weather shader
    m_WeatherShader = std::make_unique<Shader>(
        "assets/shaders/weather.vert",
        "assets/shaders/weather.frag"
    );
    
    if (!m_WeatherShader->IsValid()) {
        LOG_ERROR("Failed to load weather shader");
        return false;
    }
    
    // Load rain texture
    m_RainTexture = std::make_unique<Texture>(
        "assets/textures/environment/rain.png"
    );
    
    if (!m_RainTexture->IsValid()) {
        LOG_WARN("Failed to load rain.png (weather may not work)");
    }
    
    // Load snow texture
    m_SnowTexture = std::make_unique<Texture>(
        "assets/textures/environment/snow.png"
    );
    
    if (!m_SnowTexture->IsValid()) {
        LOG_WARN("Failed to load snow.png (weather may not work)");
    }
    
    // Generate particle positions (random within cylinder around origin)
    std::mt19937 rng(42);  // Fixed seed for consistent particles
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * static_cast<float>(M_PI));
    std::uniform_real_distribution<float> distRadius(0.0f, 32.0f);  // 32 block radius
    std::uniform_real_distribution<float> distHeight(0.0f, 128.0f);  // 128 blocks tall
    std::uniform_real_distribution<float> distOffset(0.0f, 128.0f);  // Random fall offset
    
    m_WeatherParticleCount = Voxel::Config::RAIN_DENSITY;
    
    std::vector<float> vertices;
    vertices.reserve(m_WeatherParticleCount * 6 * 6);  // 6 vertices * 6 floats each
    
    for (int i = 0; i < m_WeatherParticleCount; i++) {
        float angle = distAngle(rng);
        float radius = distRadius(rng);
        float height = distHeight(rng);
        float offset = distOffset(rng);
        
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        
        // Create a quad for this particle (2 triangles)
        // pos(3) + texCoord(2) + offset(1)
        // Triangle 1
        vertices.insert(vertices.end(), {x, height, z, 0.0f, 0.0f, offset});  // BL
        vertices.insert(vertices.end(), {x, height, z, 1.0f, 0.0f, offset});  // BR
        vertices.insert(vertices.end(), {x, height, z, 1.0f, 1.0f, offset});  // TR
        // Triangle 2
        vertices.insert(vertices.end(), {x, height, z, 0.0f, 0.0f, offset});  // BL
        vertices.insert(vertices.end(), {x, height, z, 1.0f, 1.0f, offset});  // TR
        vertices.insert(vertices.end(), {x, height, z, 0.0f, 1.0f, offset});  // TL
    }
    
    glGenVertexArrays(1, &m_WeatherVAO);
    glGenBuffers(1, &m_WeatherVBO);
    
    glBindVertexArray(m_WeatherVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_WeatherVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Offset (location 2)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    LOG_INFO("Weather system initialized with " + std::to_string(m_WeatherParticleCount) + " particles");
    return true;
}

void SkyRenderer::UpdateWeatherParticles([[maybe_unused]] const glm::vec3& cameraPos) {
    // Particles are animated in the shader, no CPU-side update needed
}

void SkyRenderer::CleanupWeather() {
    if (m_WeatherVAO != 0) {
        glDeleteVertexArrays(1, &m_WeatherVAO);
        m_WeatherVAO = 0;
    }
    if (m_WeatherVBO != 0) {
        glDeleteBuffers(1, &m_WeatherVBO);
        m_WeatherVBO = 0;
    }
}

} // namespace Core
