#include "world/RuntimeConfig.h"
#include "core/Logger.h"
#include <string>

namespace Voxel {
namespace Config {

RuntimeConfig &RuntimeConfig::Instance() {
  static RuntimeConfig instance;
  return instance;
}

void RuntimeConfig::Apply() {
  // Update fog based on render distance
  fogStart = GetFogEndForRenderDistance() - 70.0f;
  fogEnd = GetFogEndForRenderDistance();

  LOG_INFO("RuntimeConfig applied:");
  LOG_INFO("  Render Distance: " + std::to_string(renderDistance));
  LOG_INFO("  Fog: " + std::to_string(fogStart) + " - " +
           std::to_string(fogEnd));
  LOG_INFO("  Shadows: " + std::string(shadowsEnabled ? "ON" : "OFF"));
  LOG_INFO("  SSAO: " + std::string(ssaoEnabled ? "ON" : "OFF"));
  LOG_INFO("  Clouds: " + std::to_string(cloudMode));
  LOG_INFO("  Day Duration: " + std::to_string(dayDuration) + "s");
}

void RuntimeConfig::ResetToDefaults() {
  renderDistance = 20;
  fogStart = 250.0f;
  fogEnd = 320.0f;
  shadowsEnabled = true;
  ssaoEnabled = true;
  cloudMode = 2;
  dayDuration = 1200.0f;
  weatherEnabled = false;

  LOG_INFO("RuntimeConfig reset to defaults");
}

} // namespace Config
} // namespace Voxel
