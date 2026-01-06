#include "GraphicsSettings.h"

namespace Core {

GraphicsSettings &GraphicsSettings::Instance() {
  static GraphicsSettings instance;
  return instance;
}

void GraphicsSettings::Apply() {
  // Update fog distances based on render distance
  fogStart = GetFogEndForRenderDistance() * 0.75f;
  fogEnd = GetFogEndForRenderDistance();
}

void GraphicsSettings::ResetToDefaults() {
  renderDistance = 20;
  fogStart = 250.0f;
  fogEnd = 320.0f;
  shadowsEnabled = true;
  ssaoEnabled = true;
  cloudMode = 2;
}

} // namespace Core
