#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>

namespace Core {
class Texture;
}

namespace UI {

class UIRenderer;

/// Button state for hover/click detection
struct Button {
  float x, y, width, height;
  bool hovered = false;
  std::function<void()> onClick;
};

/// Minecraft-style title screen with logo and menu buttons
class TitleScreen {
public:
  TitleScreen(UIRenderer &renderer);
  ~TitleScreen();

  /// Update button hover states based on mouse position
  void Update(float mouseX, float mouseY);

  /// Handle mouse click at given position
  /// @return true if a button was clicked
  bool OnClick(float mouseX, float mouseY);

  /// Render the title screen
  void Render(int screenWidth, int screenHeight);

  /// Set callback for when "Singleplayer" is clicked
  void SetOnPlay(std::function<void()> callback) { m_OnPlay = callback; }

  /// Set callback for when "Quit" is clicked
  void SetOnQuit(std::function<void()> callback) { m_OnQuit = callback; }

  /// Set callback for when "Options" is clicked
  void SetOnOptions(std::function<void()> callback) { m_OnOptions = callback; }

private:
  void LoadTextures();
  void UpdateButtonLayout(int screenWidth, int screenHeight);

  UIRenderer &m_Renderer;

  // Textures
  std::unique_ptr<Core::Texture> m_BackgroundTex;
  std::unique_ptr<Core::Texture> m_LogoTex;
  std::unique_ptr<Core::Texture> m_ButtonTex;
  std::unique_ptr<Core::Texture> m_ButtonHoverTex;

  // Text textures for button labels
  std::unique_ptr<Core::Texture> m_SingleplayerTextTex;
  std::unique_ptr<Core::Texture> m_OptionsTextTex;
  std::unique_ptr<Core::Texture> m_QuitTextTex;

  // Buttons
  Button m_PlayButton;
  Button m_OptionsButton;
  Button m_QuitButton;

  // Callbacks
  std::function<void()> m_OnPlay;
  std::function<void()> m_OnOptions;
  std::function<void()> m_OnQuit;

  // Layout cache
  int m_LastWidth = 0;
  int m_LastHeight = 0;
};

} // namespace UI
