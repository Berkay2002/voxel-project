#pragma once

namespace Core {

/// Game state enumeration for controlling the main loop flow
enum class GameState {
  TITLE_SCREEN, // Main menu with play/quit buttons
  SETTINGS,     // Options/settings menu
  LOADING,      // World generation and loading
  PLAYING       // Normal gameplay mode
};

} // namespace Core
