#include "world/GameConfig.h"

namespace Voxel {
namespace Config {

GameConfig &GameConfig::Instance() {
  static GameConfig instance;
  return instance;
}

void GameConfig::Apply() {
  // Game-specific settings don't need immediate application
  // They are read each frame by the relevant systems
}

void GameConfig::ResetToDefaults() {
  dayDuration = 1200.0f;
  weatherEnabled = false;
}

} // namespace Config
} // namespace Voxel
