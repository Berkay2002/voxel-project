#include "core/RenderConfig.h"

namespace Core {

RenderConfig &RenderConfig::Instance() {
  static RenderConfig instance;
  return instance;
}

void RenderConfig::Apply() {
  // Update fog distances based on render distance
  fogStart = GetFogEndForRenderDistance() * 0.75f;
  fogEnd = GetFogEndForRenderDistance();
}

void RenderConfig::ResetToDefaults() {
  renderDistance = 20;
  fogStart = 250.0f;
  fogEnd = 320.0f;
  shadowsEnabled = true;
  ssaoEnabled = true;
  cloudMode = 2;
}

} // namespace Core
