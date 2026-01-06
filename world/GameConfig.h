#pragma once

/**
 * Game Configuration
 *
 * Singleton class for game/world-specific settings that can be modified at runtime.
 * Rendering settings are in core/GraphicsSettings.h for proper layer separation.
 */

namespace Voxel {
namespace Config {

class GameConfig {
public:
  /// Get the singleton instance
  static GameConfig &Instance();

  // Non-copyable
  GameConfig(const GameConfig &) = delete;
  GameConfig &operator=(const GameConfig &) = delete;

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

  /// Apply current settings (call after changing values)
  void Apply();

  /// Reset all settings to defaults
  void ResetToDefaults();

private:
  GameConfig() = default;
};



} // namespace Config
} // namespace Voxel
