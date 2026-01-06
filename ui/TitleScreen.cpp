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

  // Load button text textures
  m_SingleplayerTextTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/text/singleplayer.png");
  m_OptionsTextTex =
      std::make_unique<Core::Texture>("assets/textures/gui/text/options.png");
  m_QuitTextTex =
      std::make_unique<Core::Texture>("assets/textures/gui/text/quit_game.png");

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

  // Position buttons below logo (70% down the screen)
  float buttonsY = screenHeight * 0.70f;

  // Play button
  m_PlayButton.x = centerX;
  m_PlayButton.y = buttonsY;
  m_PlayButton.width = buttonWidth;
  m_PlayButton.height = buttonHeight;

  // Options button (below play)
  m_OptionsButton.x = centerX;
  m_OptionsButton.y = buttonsY + buttonHeight + buttonSpacing;
  m_OptionsButton.width = buttonWidth;
  m_OptionsButton.height = buttonHeight;

  // Quit button (below options)
  m_QuitButton.x = centerX;
  m_QuitButton.y = buttonsY + 2 * (buttonHeight + buttonSpacing);
  m_QuitButton.width = buttonWidth;
  m_QuitButton.height = buttonHeight;
}

void TitleScreen::Update(float mouseX, float mouseY) {
  // Check play button hover
  m_PlayButton.hovered = mouseX >= m_PlayButton.x &&
                         mouseX <= m_PlayButton.x + m_PlayButton.width &&
                         mouseY >= m_PlayButton.y &&
                         mouseY <= m_PlayButton.y + m_PlayButton.height;

  // Check options button hover
  m_OptionsButton.hovered =
      mouseX >= m_OptionsButton.x &&
      mouseX <= m_OptionsButton.x + m_OptionsButton.width &&
      mouseY >= m_OptionsButton.y &&
      mouseY <= m_OptionsButton.y + m_OptionsButton.height;

  // Check quit button hover
  m_QuitButton.hovered = mouseX >= m_QuitButton.x &&
                         mouseX <= m_QuitButton.x + m_QuitButton.width &&
                         mouseY >= m_QuitButton.y &&
                         mouseY <= m_QuitButton.y + m_QuitButton.height;
}

bool TitleScreen::OnClick(float mouseX, float mouseY) {
  (void)mouseX;
  (void)mouseY;

  // Check play button
  if (m_PlayButton.hovered && m_OnPlay) {
    m_OnPlay();
    return true;
  }

  // Check options button
  if (m_OptionsButton.hovered && m_OnOptions) {
    m_OnOptions();
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
  if (m_BackgroundTex && m_BackgroundTex->IsValid()) {
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
  if (m_LogoTex && m_LogoTex->IsValid()) {
    // Use actual texture dimensions for proper aspect ratio
    float texWidth = static_cast<float>(m_LogoTex->GetWidth());
    float texHeight = static_cast<float>(m_LogoTex->GetHeight());
    float aspectRatio = texWidth / texHeight;

    // Scale to fit nicely on screen (max width 350px for good spacing)
    float logoWidth = 350.0f;
    float logoHeight = logoWidth / aspectRatio;
    float logoX = (screenWidth - logoWidth) / 2.0f;
    float logoY = screenHeight * 0.15f;

    m_Renderer.DrawTexture(*m_LogoTex, logoX, logoY, logoWidth, logoHeight);
  }

  // Draw Play button
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &playTex =
        m_PlayButton.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (playTex.IsValid()) {
      m_Renderer.DrawTexture(playTex, m_PlayButton.x, m_PlayButton.y,
                             m_PlayButton.width, m_PlayButton.height);
    }

    // Draw "Singleplayer" text overlay
    if (m_SingleplayerTextTex && m_SingleplayerTextTex->IsValid()) {
      float texW = static_cast<float>(m_SingleplayerTextTex->GetWidth());
      float texH = static_cast<float>(m_SingleplayerTextTex->GetHeight());
      // Use fixed pixel width for readability
      float targetWidth = 200.0f;
      float scale = targetWidth / texW;
      float textWidth = texW * scale;
      float textHeight = texH * scale;
      float textX = m_PlayButton.x + (m_PlayButton.width - textWidth) / 2.0f;
      float textY = m_PlayButton.y + (m_PlayButton.height - textHeight) / 2.0f;
      m_Renderer.DrawTexture(*m_SingleplayerTextTex, textX, textY, textWidth,
                             textHeight);
    }
  }

  // Draw Options button
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &optionsTex =
        m_OptionsButton.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (optionsTex.IsValid()) {
      m_Renderer.DrawTexture(optionsTex, m_OptionsButton.x, m_OptionsButton.y,
                             m_OptionsButton.width, m_OptionsButton.height);
    }

    // Draw "Options" text overlay
    if (m_OptionsTextTex && m_OptionsTextTex->IsValid()) {
      float texW = static_cast<float>(m_OptionsTextTex->GetWidth());
      float texH = static_cast<float>(m_OptionsTextTex->GetHeight());
      // Use fixed pixel width for readability
      float targetWidth = 140.0f;
      float scale = targetWidth / texW;
      float textWidth = texW * scale;
      float textHeight = texH * scale;
      float textX =
          m_OptionsButton.x + (m_OptionsButton.width - textWidth) / 2.0f;
      float textY =
          m_OptionsButton.y + (m_OptionsButton.height - textHeight) / 2.0f;
      m_Renderer.DrawTexture(*m_OptionsTextTex, textX, textY, textWidth,
                             textHeight);
    }
  }

  // Draw Quit button
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &quitTex =
        m_QuitButton.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (quitTex.IsValid()) {
      m_Renderer.DrawTexture(quitTex, m_QuitButton.x, m_QuitButton.y,
                             m_QuitButton.width, m_QuitButton.height);
    }

    // Draw "Quit Game" text overlay
    if (m_QuitTextTex && m_QuitTextTex->IsValid()) {
      float texW = static_cast<float>(m_QuitTextTex->GetWidth());
      float texH = static_cast<float>(m_QuitTextTex->GetHeight());
      // Use fixed pixel width for readability
      float targetWidth = 170.0f;
      float scale = targetWidth / texW;
      float textWidth = texW * scale;
      float textHeight = texH * scale;
      float textX = m_QuitButton.x + (m_QuitButton.width - textWidth) / 2.0f;
      float textY = m_QuitButton.y + (m_QuitButton.height - textHeight) / 2.0f;
      m_Renderer.DrawTexture(*m_QuitTextTex, textX, textY, textWidth,
                             textHeight);
    }
  }

  m_Renderer.End();
}

} // namespace UI
