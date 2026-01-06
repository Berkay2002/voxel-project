#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <FastNoiseLite.h>

namespace Core {
class Shader;
class Texture;
class Camera;

/**
 * SkySystem - Modular sky system for clouds, celestials, and weather.
 * 
 * Design Principles:
 * - MODULARITY: Each subsystem (clouds, sun/moon, weather) can be independently enabled/disabled
 * - SCALABILITY: Weather uses instanced rendering for many particles
 * - CONFIGURABILITY: All parameters come from WorldConfig.h
 */
class SkySystem {
public:
    SkySystem();
    ~SkySystem();

    // Disable copy, allow move
    SkySystem(const SkySystem&) = delete;
    SkySystem& operator=(const SkySystem&) = delete;
    SkySystem(SkySystem&&) noexcept = default;
    SkySystem& operator=(SkySystem&&) noexcept = default;

    /**
     * Initialize all sky subsystems (load shaders, create meshes, load textures)
     * @return true if setup succeeded
     */
    bool Setup();

    /**
     * Update time-of-day, cloud drift, weather particles
     * @param deltaTime Frame time in seconds
     * @param cameraPos Player position for centering weather/clouds
     */
    void Update(float deltaTime, const glm::vec3& cameraPos);

    /**
     * Render the sky (clouds, sun, moon)
     * Should be called BEFORE terrain, with depth write disabled
     */
    void Render(const Camera& camera, float aspectRatio);

    /**
     * Render weather particles (rain/snow)
     * Should be called AFTER terrain with blending enabled
     */
    void RenderWeather(const Camera& camera, float aspectRatio);

    // =========================================================================
    // TIME OF DAY
    // =========================================================================

    /**
     * Get normalized time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
     */
    [[nodiscard]] float GetTimeOfDay() const { return m_TimeOfDay; }

    /**
     * Set time of day manually (for debugging/testing)
     */
    void SetTimeOfDay(float time);

    /**
     * Get sky color based on current time (for glClearColor)
     */
    [[nodiscard]] glm::vec3 GetSkyColor() const;

    /**
     * Get sun direction for lighting (normalized)
     */
    [[nodiscard]] glm::vec3 GetSunDirection() const;

    /**
     * Get ambient strength based on time of day
     */
    [[nodiscard]] float GetAmbientStrength() const;

    // =========================================================================
    // FEATURE TOGGLES
    // =========================================================================

    void SetCloudsEnabled(bool enabled) { m_CloudsEnabled = enabled; }
    void SetCelestialsEnabled(bool enabled) { m_CelestialsEnabled = enabled; }
    void SetWeatherEnabled(bool enabled) { m_WeatherEnabled = enabled; }

    [[nodiscard]] bool IsCloudsEnabled() const { return m_CloudsEnabled; }
    [[nodiscard]] bool IsCelestialsEnabled() const { return m_CelestialsEnabled; }
    [[nodiscard]] bool IsWeatherEnabled() const { return m_WeatherEnabled; }

    /**
     * Toggle weather state (called from key press)
     */
    void ToggleWeather() { m_WeatherEnabled = !m_WeatherEnabled; }

    // =========================================================================
    // WEATHER TYPE
    // =========================================================================

    enum class WeatherType { None, Rain, Snow };
    void SetWeatherType(WeatherType type) { m_WeatherType = type; }
    [[nodiscard]] WeatherType GetWeatherType() const { return m_WeatherType; }

private:
    // =========================================================================
    // TIME SYSTEM
    // =========================================================================
    
    float m_TimeOfDay = 0.25f;  // Start at dawn (6:00 AM)
    int m_DayCount = 0;         // For moon phase cycling
    
    // =========================================================================
    // CLOUD LAYER (Phase 14A) - Fast 2D mode
    // =========================================================================
    
    bool m_CloudsEnabled = true;
    float m_CloudOffset = 0.0f;  // UV offset for drift animation
    
    std::unique_ptr<Shader> m_CloudShader;
    std::unique_ptr<Texture> m_CloudTexture;
    unsigned int m_CloudVAO = 0;
    unsigned int m_CloudVBO = 0;
    unsigned int m_CloudIBO = 0;
    int m_CloudIndexCount = 0;
    
    bool SetupClouds();
    void RenderClouds(const Camera& camera, float aspectRatio);
    void CleanupClouds();
    
    // =========================================================================
    // VOLUMETRIC CLOUDS (Phase 14E) - Fancy 3D mode
    // =========================================================================
    
    std::unique_ptr<Shader> m_VolumetricCloudShader;
    unsigned int m_VolumetricCloudVAO = 0;
    unsigned int m_VolumetricCloudVBO = 0;
    int m_VolumetricCloudVertexCount = 0;
    glm::ivec2 m_LastCloudGridCenter{0, 0};  // For mesh caching
    
    // FastNoiseLite for organic cloud shapes (replaces sin/cos noise)
    std::unique_ptr<FastNoiseLite> m_CloudNoise;
    
    bool SetupVolumetricClouds();
    void RenderVolumetricClouds(const Camera& camera, float aspectRatio);
    void RebuildCloudMesh(int centerX, int centerZ);
    void CleanupVolumetricClouds();
    
    /**
     * Sample cloud texture for occupancy at grid coordinates
     * @return true if cloud cell is occupied (alpha > threshold)
     */
    bool IsCloudOccupied(int gridX, int gridZ) const;
    
    // =========================================================================
    // CELESTIALS - SUN & MOON (Phase 14B)
    // =========================================================================
    
    bool m_CelestialsEnabled = true;
    
    std::unique_ptr<Shader> m_CelestialShader;
    std::unique_ptr<Texture> m_SunTexture;
    std::unique_ptr<Texture> m_MoonTexture;
    unsigned int m_BillboardVAO = 0;
    unsigned int m_BillboardVBO = 0;
    
    bool SetupCelestials();
    void RenderCelestials(const Camera& camera, float aspectRatio);
    void CleanupCelestials();
    
    /**
     * Get moon phase index (0-7) based on day count
     */
    [[nodiscard]] int GetMoonPhase() const { return m_DayCount % 8; }
    
    // =========================================================================
    // WEATHER SYSTEM (Phase 14D)
    // =========================================================================
    
    bool m_WeatherEnabled = false;  // Start disabled
    WeatherType m_WeatherType = WeatherType::Rain;
    
    std::unique_ptr<Shader> m_WeatherShader;
    std::unique_ptr<Texture> m_RainTexture;
    std::unique_ptr<Texture> m_SnowTexture;
    unsigned int m_WeatherVAO = 0;
    unsigned int m_WeatherVBO = 0;
    int m_WeatherParticleCount = 0;
    
    glm::vec3 m_LastCameraPos = glm::vec3(0.0f);  // For repositioning particles
    
    bool SetupWeather();
    void UpdateWeatherParticles(const glm::vec3& cameraPos);
    void CleanupWeather();
};

} // namespace Core
