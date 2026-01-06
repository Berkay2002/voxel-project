#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Core {
class Texture;
}

namespace UI {

class UIRenderer;

/// Button state for hover/click detection
struct SettingsButton {
  float x, y, width, height;
  bool hovered = false;
  std::function<void()> onClick;
};

/// A single settings row with label area, value display, and +/- buttons
struct SettingRow {
  std::string label; // For logging/debugging
  float y;           // Y position of row

  // Value range
  float value;
  float minVal;
  float maxVal;
  float step;

  // +/- buttons
  SettingsButton decreaseBtn;
  SettingsButton increaseBtn;

  // Progress bar area (between buttons)
  float barX, barWidth;
};

/// Minecraft-style settings screen with adjustable options
class SettingsScreen {
public:
  SettingsScreen(UIRenderer &renderer);
  ~SettingsScreen();

  /// Update button hover states based on mouse position
  void Update(float mouseX, float mouseY);

  /// Handle mouse click at given position
  /// @return true if a button was clicked
  bool OnClick(float mouseX, float mouseY);

  /// Render the settings screen
  void Render(int screenWidth, int screenHeight);

  /// Set callback for when "Done" is clicked
  void SetOnBack(std::function<void()> callback) { m_OnBack = callback; }

private:
  void LoadTextures();
  void UpdateLayout(int screenWidth, int screenHeight);
  void SyncFromConfig(); // Load current values from RuntimeConfig
  void SyncToConfig();   // Save current values to RuntimeConfig
  void DrawSettingRow(int screenWidth, const SettingRow &row,
                      const std::string &valueText);

  UIRenderer &m_Renderer;

  // Textures
  std::unique_ptr<Core::Texture> m_BackgroundTex;
  std::unique_ptr<Core::Texture> m_ButtonTex;
  std::unique_ptr<Core::Texture> m_ButtonHoverTex;

  // Text textures for labels
  std::unique_ptr<Core::Texture> m_RenderDistanceText;
  std::unique_ptr<Core::Texture> m_ShadowsText;
  std::unique_ptr<Core::Texture> m_SSAOText;
  std::unique_ptr<Core::Texture> m_CloudsText;
  std::unique_ptr<Core::Texture> m_DayCycleText;
  std::unique_ptr<Core::Texture> m_DoneText;
  std::unique_ptr<Core::Texture> m_TitleOptionsText;

  // Setting rows
  SettingRow m_RenderDistance;
  SettingRow m_Shadows;
  SettingRow m_SSAO;
  SettingRow m_Clouds;
  SettingRow m_DayCycleSpeed;

  // Back button
  SettingsButton m_BackButton;

  // Callbacks
  std::function<void()> m_OnBack;

  // Layout cache
  int m_LastWidth = 0;
  int m_LastHeight = 0;
};

} // namespace UI
