#pragma once

/**
 * Runtime Configuration
 *
 * Singleton class for settings that can be modified at runtime.
 * These override the compile-time defaults in WorldConfig.h.
 */

namespace Voxel {
namespace Config {

class RuntimeConfig {
public:
  /// Get the singleton instance
  static RuntimeConfig &Instance();

  // Non-copyable
  RuntimeConfig(const RuntimeConfig &) = delete;
  RuntimeConfig &operator=(const RuntimeConfig &) = delete;

  // =========================================================================
  // RENDERING SETTINGS
  // =========================================================================

  /// Chunk load radius (affects how far you can see)
  /// Default: 20 (from CHUNK_LOAD_RADIUS)
  int renderDistance = 20;

  /// Fog start distance (blocks)
  float fogStart = 250.0f;

  /// Fog end distance (blocks)
  float fogEnd = 320.0f;

  /// Enable shadow mapping
  bool shadowsEnabled = true;

  /// Enable Screen-Space Ambient Occlusion
  bool ssaoEnabled = true;

  /// Cloud rendering mode: 0=OFF, 1=FAST (2D), 2=FANCY (3D volumetric)
  int cloudMode = 2;

  // =========================================================================
  // TIME & WEATHER
  // =========================================================================

  /// Day duration in seconds (default: 1200 = 20 minutes)
  float dayDuration = 1200.0f;

  /// Weather enabled (rain/snow particles)
  bool weatherEnabled = false;

  // =========================================================================
  // HELPER METHODS
  // =========================================================================

  /// Calculate fog end distance based on render distance
  /// Roughly: (renderDistance * 16) blocks visibility
  float GetFogEndForRenderDistance() const {
    return static_cast<float>(renderDistance) * 16.0f;
  }

  /// Get unload radius (slightly larger than load radius)
  int GetUnloadRadius() const { return renderDistance + 2; }

  /// Apply current settings (call after changing values)
  void Apply();

  /// Reset all settings to defaults
  void ResetToDefaults();

private:
  RuntimeConfig() = default;
};

} // namespace Config
} // namespace Voxel
