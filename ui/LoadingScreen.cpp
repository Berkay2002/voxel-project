#include "ui/LoadingScreen.h"
#include "core/Logger.h"
#include "core/Texture.h"
#include "ui/UIRenderer.h"

#include <algorithm>

namespace UI {

LoadingScreen::LoadingScreen(UIRenderer &renderer) : m_Renderer(renderer) {
  LoadTextures();
  LOG_INFO("LoadingScreen initialized");
}

LoadingScreen::~LoadingScreen() = default;

void LoadingScreen::LoadTextures() {
  // Load background
  m_BackgroundTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/options_background.png");

  // Load logo
  m_LogoTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/title/voxelcraft_logo.png");

  LOG_INFO("LoadingScreen textures loaded");
}

void LoadingScreen::SetProgress(float progress) {
  m_Progress = std::clamp(progress, 0.0f, 1.0f);
}

void LoadingScreen::SetStatus(const std::string &status) { m_Status = status; }

void LoadingScreen::Render(int screenWidth, int screenHeight) {
  m_Renderer.SetScreenSize(screenWidth, screenHeight);
  m_Renderer.Begin();

  // Draw tiled background (darker)
  if (m_BackgroundTex && m_BackgroundTex->IsValid()) {
    float tileSize = 64.0f;
    m_Renderer.DrawTiled(*m_BackgroundTex, 0, 0,
                         static_cast<float>(screenWidth),
                         static_cast<float>(screenHeight), tileSize, tileSize);

    // Dark overlay
    m_Renderer.DrawRect(0, 0, static_cast<float>(screenWidth),
                        static_cast<float>(screenHeight),
                        glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
  }

  // Draw logo (centered, upper portion) - same sizing as TitleScreen
  if (m_LogoTex && m_LogoTex->IsValid()) {
    float texWidth = static_cast<float>(m_LogoTex->GetWidth());
    float texHeight = static_cast<float>(m_LogoTex->GetHeight());
    float aspectRatio = texWidth / texHeight;

    float logoWidth = 350.0f;
    float logoHeight = logoWidth / aspectRatio;
    float logoX = (screenWidth - logoWidth) / 2.0f;
    float logoY = screenHeight * 0.10f;

    m_Renderer.DrawTexture(*m_LogoTex, logoX, logoY, logoWidth, logoHeight);
  }

  // Progress bar dimensions
  float barWidth = 400.0f;
  float barHeight = 20.0f;
  float barX = (screenWidth - barWidth) / 2.0f;
  float barY = screenHeight * 0.6f;

  // Progress bar background (dark gray)
  m_Renderer.DrawRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4,
                      glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

  // Progress bar border (light gray)
  m_Renderer.DrawRect(barX - 1, barY - 1, barWidth + 2, barHeight + 2,
                      glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

  // Progress bar background (black)
  m_Renderer.DrawRect(barX, barY, barWidth, barHeight,
                      glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));

  // Progress bar fill (green gradient effect)
  if (m_Progress > 0.0f) {
    float fillWidth = barWidth * m_Progress;
    // Main green fill
    m_Renderer.DrawRect(barX, barY, fillWidth, barHeight,
                        glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
    // Lighter top highlight
    m_Renderer.DrawRect(barX, barY, fillWidth, barHeight * 0.3f,
                        glm::vec4(0.4f, 1.0f, 0.4f, 0.5f));
  }

  // Loading percentage text placeholder (white rect)
  float percentWidth = 50.0f;
  float percentHeight = 16.0f;
  float percentX = (screenWidth - percentWidth) / 2.0f;
  float percentY = barY + barHeight + 15.0f;
  m_Renderer.DrawRect(percentX, percentY, percentWidth, percentHeight,
                      glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));

  // Status text placeholder (below percentage)
  float statusWidth = 200.0f;
  float statusHeight = 14.0f;
  float statusX = (screenWidth - statusWidth) / 2.0f;
  float statusY = percentY + percentHeight + 10.0f;
  m_Renderer.DrawRect(statusX, statusY, statusWidth, statusHeight,
                      glm::vec4(0.8f, 0.8f, 0.8f, 0.6f));

  m_Renderer.End();
}

} // namespace UI
