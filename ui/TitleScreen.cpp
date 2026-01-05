#include "ui/TitleScreen.h"
#include "core/Logger.h"
#include "core/Texture.h"
#include "ui/UIRenderer.h"


namespace UI {

TitleScreen::TitleScreen(UIRenderer &renderer) : m_Renderer(renderer) {
  LoadTextures();
  LOG_INFO("TitleScreen initialized");
}

TitleScreen::~TitleScreen() = default;

void TitleScreen::LoadTextures() {
  // Load background (dirt texture for tiling)
  m_BackgroundTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/options_background.png");

  // Load VoxelCraft logo
  m_LogoTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/title/voxelcraft_logo.png");

  // Load button textures
  m_ButtonTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/sprites/widget/button.png");
  m_ButtonHoverTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/sprites/widget/button_highlighted.png");

  LOG_INFO("TitleScreen textures loaded");
}

void TitleScreen::UpdateButtonLayout(int screenWidth, int screenHeight) {
  if (screenWidth == m_LastWidth && screenHeight == m_LastHeight)
    return;

  m_LastWidth = screenWidth;
  m_LastHeight = screenHeight;

  // Button dimensions (Minecraft style: 200x20 base, scaled for HD)
  float buttonWidth = 400.0f;
  float buttonHeight = 40.0f;
  float buttonSpacing = 10.0f;

  // Center buttons horizontally
  float centerX = (screenWidth - buttonWidth) / 2.0f;

  // Position buttons below logo (roughly 60% down the screen)
  float buttonsY = screenHeight * 0.55f;

  // Play button
  m_PlayButton.x = centerX;
  m_PlayButton.y = buttonsY;
  m_PlayButton.width = buttonWidth;
  m_PlayButton.height = buttonHeight;

  // Quit button (below play)
  m_QuitButton.x = centerX;
  m_QuitButton.y = buttonsY + buttonHeight + buttonSpacing;
  m_QuitButton.width = buttonWidth;
  m_QuitButton.height = buttonHeight;
}

void TitleScreen::Update(float mouseX, float mouseY) {
  // Check play button hover
  m_PlayButton.hovered = mouseX >= m_PlayButton.x &&
                         mouseX <= m_PlayButton.x + m_PlayButton.width &&
                         mouseY >= m_PlayButton.y &&
                         mouseY <= m_PlayButton.y + m_PlayButton.height;

  // Check quit button hover
  m_QuitButton.hovered = mouseX >= m_QuitButton.x &&
                         mouseX <= m_QuitButton.x + m_QuitButton.width &&
                         mouseY >= m_QuitButton.y &&
                         mouseY <= m_QuitButton.y + m_QuitButton.height;
}

bool TitleScreen::OnClick(float mouseX, float mouseY) {
  // Check play button
  if (m_PlayButton.hovered && m_OnPlay) {
    m_OnPlay();
    return true;
  }

  // Check quit button
  if (m_QuitButton.hovered && m_OnQuit) {
    m_OnQuit();
    return true;
  }

  return false;
}

void TitleScreen::Render(int screenWidth, int screenHeight) {
  UpdateButtonLayout(screenWidth, screenHeight);

  m_Renderer.SetScreenSize(screenWidth, screenHeight);
  m_Renderer.Begin();

  // Draw tiled background (darker tint for better contrast)
  if (m_BackgroundTex && m_BackgroundTex->IsLoaded()) {
    // Tile size matches texture (typically 64x64 for Minecraft backgrounds)
    float tileSize = 64.0f;
    m_Renderer.DrawTiled(*m_BackgroundTex, 0, 0,
                         static_cast<float>(screenWidth),
                         static_cast<float>(screenHeight), tileSize, tileSize);

    // Apply dark overlay for better logo visibility
    m_Renderer.DrawRect(0, 0, static_cast<float>(screenWidth),
                        static_cast<float>(screenHeight),
                        glm::vec4(0.0f, 0.0f, 0.0f, 0.4f));
  }

  // Draw logo (centered at top)
  if (m_LogoTex && m_LogoTex->IsLoaded()) {
    float logoWidth = 600.0f;
    float logoHeight = 150.0f; // Maintain aspect ratio
    float logoX = (screenWidth - logoWidth) / 2.0f;
    float logoY = screenHeight * 0.12f;

    m_Renderer.DrawTexture(*m_LogoTex, logoX, logoY, logoWidth, logoHeight);
  }

  // Draw Play button
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &playTex =
        m_PlayButton.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (playTex.IsLoaded()) {
      m_Renderer.DrawTexture(playTex, m_PlayButton.x, m_PlayButton.y,
                             m_PlayButton.width, m_PlayButton.height);
    }

    // Draw "Singleplayer" text overlay (center of button) - using colored rect
    // as placeholder In a full implementation, you'd use font rendering here
    float textWidth = 120.0f;
    float textHeight = 12.0f;
    float textX = m_PlayButton.x + (m_PlayButton.width - textWidth) / 2.0f;
    float textY = m_PlayButton.y + (m_PlayButton.height - textHeight) / 2.0f;
    m_Renderer.DrawRect(textX, textY, textWidth, textHeight,
                        glm::vec4(1.0f, 1.0f, 1.0f, 0.9f));
  }

  // Draw Quit button
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &quitTex =
        m_QuitButton.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (quitTex.IsLoaded()) {
      m_Renderer.DrawTexture(quitTex, m_QuitButton.x, m_QuitButton.y,
                             m_QuitButton.width, m_QuitButton.height);
    }

    // Draw "Quit Game" text placeholder
    float textWidth = 80.0f;
    float textHeight = 12.0f;
    float textX = m_QuitButton.x + (m_QuitButton.width - textWidth) / 2.0f;
    float textY = m_QuitButton.y + (m_QuitButton.height - textHeight) / 2.0f;
    m_Renderer.DrawRect(textX, textY, textWidth, textHeight,
                        glm::vec4(1.0f, 1.0f, 1.0f, 0.9f));
  }

  m_Renderer.End();
}

} // namespace UI
