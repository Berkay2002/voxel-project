#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Core {
class Texture;
}

namespace UI {

class UIRenderer;

/// Loading screen with progress bar and status text
class LoadingScreen {
public:
  LoadingScreen(UIRenderer &renderer);
  ~LoadingScreen();

  /// Set loading progress (0.0 to 1.0)
  void SetProgress(float progress);

  /// Set loading status message
  void SetStatus(const std::string &status);

  /// Check if loading is complete (progress >= 1.0)
  bool IsComplete() const { return m_Progress >= 1.0f; }

  /// Render the loading screen
  void Render(int screenWidth, int screenHeight);

private:
  void LoadTextures();

  UIRenderer &m_Renderer;

  // Textures
  std::unique_ptr<Core::Texture> m_BackgroundTex;
  std::unique_ptr<Core::Texture> m_LogoTex;

  // State
  float m_Progress = 0.0f;
  std::string m_Status = "Loading...";
};

} // namespace UI
