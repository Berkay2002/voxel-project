#pragma once

/**
 * VideoSettings
 *
 * Singleton class for display/window settings that can be modified at runtime.
 * Separated from GameConfig (world/) as display belongs to core engine layer.
 */

namespace Core {

/// Window display modes (like Minecraft's Video Settings)
enum class WindowMode {
  WINDOWED = 0,
  BORDERLESS_FULLSCREEN = 1,
  FULLSCREEN = 2
};

class VideoSettings {
public:
  /// Get the singleton instance
  static VideoSettings &Instance();

  // Non-copyable
  VideoSettings(const VideoSettings &) = delete;
  VideoSettings &operator=(const VideoSettings &) = delete;

  // =========================================================================
  // WINDOW SETTINGS
  // =========================================================================

  /// Current window mode
  WindowMode windowMode = WindowMode::WINDOWED;

  /// Windowed mode size (remembered when switching to fullscreen)
  int windowedWidth = 1280;
  int windowedHeight = 720;

  /// Windowed mode position (remembered when switching to fullscreen)
  int windowPosX = 100;
  int windowPosY = 100;

  /// VSync enabled
  bool vsyncEnabled = true;

  // =========================================================================
  // HELPER METHODS
  // =========================================================================

  /// Get display mode name for UI
  static const char *GetWindowModeName(WindowMode mode) {
    switch (mode) {
    case WindowMode::WINDOWED:
      return "Windowed";
    case WindowMode::BORDERLESS_FULLSCREEN:
      return "Borderless";
    case WindowMode::FULLSCREEN:
      return "Fullscreen";
    default:
      return "Unknown";
    }
  }

private:
  VideoSettings() = default;
};

} // namespace Core
