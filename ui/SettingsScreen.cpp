#include "ui/SettingsScreen.h"
#include "core/Logger.h"
#include "core/Texture.h"
#include "ui/UIRenderer.h"
#include "world/RuntimeConfig.h"

namespace UI {

SettingsScreen::SettingsScreen(UIRenderer &renderer) : m_Renderer(renderer) {
  LoadTextures();
  SyncFromConfig();
  LOG_INFO("SettingsScreen initialized");
}

SettingsScreen::~SettingsScreen() = default;

void SettingsScreen::LoadTextures() {
  // Reuse title screen textures
  m_BackgroundTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/options_background.png");
  m_ButtonTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/sprites/widget/button.png");
  m_ButtonHoverTex = std::make_unique<Core::Texture>(
      "assets/textures/gui/sprites/widget/button_highlighted.png");

  // Load label text textures
  m_RenderDistanceText = std::make_unique<Core::Texture>(
      "assets/textures/gui/text/render_distance.png");
  m_ShadowsText =
      std::make_unique<Core::Texture>("assets/textures/gui/text/shadows.png");
  m_SSAOText =
      std::make_unique<Core::Texture>("assets/textures/gui/text/ssao.png");
  m_CloudsText =
      std::make_unique<Core::Texture>("assets/textures/gui/text/clouds.png");
  m_DayCycleText =
      std::make_unique<Core::Texture>("assets/textures/gui/text/day_cycle.png");
  m_DoneText =
      std::make_unique<Core::Texture>("assets/textures/gui/text/done.png");
  m_TitleOptionsText = std::make_unique<Core::Texture>(
      "assets/textures/gui/text/title_options.png");

  LOG_INFO("SettingsScreen textures loaded");
}

void SettingsScreen::SyncFromConfig() {
  auto &config = Voxel::Config::RuntimeConfig::Instance();

  m_RenderDistance.label = "Render Distance";
  m_RenderDistance.value = static_cast<float>(config.renderDistance);
  m_RenderDistance.minVal = 4.0f;
  m_RenderDistance.maxVal = 32.0f;
  m_RenderDistance.step = 2.0f;

  m_Shadows.label = "Shadows";
  m_Shadows.value = config.shadowsEnabled ? 1.0f : 0.0f;
  m_Shadows.minVal = 0.0f;
  m_Shadows.maxVal = 1.0f;
  m_Shadows.step = 1.0f;

  m_SSAO.label = "SSAO";
  m_SSAO.value = config.ssaoEnabled ? 1.0f : 0.0f;
  m_SSAO.minVal = 0.0f;
  m_SSAO.maxVal = 1.0f;
  m_SSAO.step = 1.0f;

  m_Clouds.label = "Clouds";
  m_Clouds.value = static_cast<float>(config.cloudMode);
  m_Clouds.minVal = 0.0f;
  m_Clouds.maxVal = 2.0f;
  m_Clouds.step = 1.0f;

  m_DayCycleSpeed.label = "Day Cycle";
  // Convert duration to speed multiplier (1200 = 1x, 600 = 2x, 2400 = 0.5x)
  m_DayCycleSpeed.value = 1200.0f / config.dayDuration;
  m_DayCycleSpeed.minVal = 0.25f; // 4x slower
  m_DayCycleSpeed.maxVal = 4.0f;  // 4x faster
  m_DayCycleSpeed.step = 0.25f;
}

void SettingsScreen::SyncToConfig() {
  auto &config = Voxel::Config::RuntimeConfig::Instance();

  config.renderDistance = static_cast<int>(m_RenderDistance.value);
  config.shadowsEnabled = m_Shadows.value > 0.5f;
  config.ssaoEnabled = m_SSAO.value > 0.5f;
  config.cloudMode = static_cast<int>(m_Clouds.value);
  config.dayDuration = 1200.0f / m_DayCycleSpeed.value;

  config.Apply();
}

void SettingsScreen::UpdateLayout(int screenWidth, int screenHeight) {
  if (screenWidth == m_LastWidth && screenHeight == m_LastHeight)
    return;

  m_LastWidth = screenWidth;
  m_LastHeight = screenHeight;

  // Layout constants
  float rowHeight = 40.0f;
  float rowSpacing = 10.0f;
  float buttonSize = 40.0f;                           // Square +/- buttons
  float rowWidth = 400.0f;                            // Total row width
  float barWidth = rowWidth - buttonSize * 2 - 20.0f; // Space between buttons

  float centerX = (screenWidth - rowWidth) / 2.0f;
  float startY = screenHeight * 0.20f; // Start rows 20% down

  // Helper to setup a row
  auto setupRow = [&](SettingRow &row, int index) {
    row.y = startY + index * (rowHeight + rowSpacing);

    // Decrease button (left)
    row.decreaseBtn.x = centerX;
    row.decreaseBtn.y = row.y;
    row.decreaseBtn.width = buttonSize;
    row.decreaseBtn.height = rowHeight;

    // Progress bar (center)
    row.barX = centerX + buttonSize + 10.0f;
    row.barWidth = barWidth;

    // Increase button (right)
    row.increaseBtn.x = centerX + rowWidth - buttonSize;
    row.increaseBtn.y = row.y;
    row.increaseBtn.width = buttonSize;
    row.increaseBtn.height = rowHeight;
  };

  setupRow(m_RenderDistance, 0);
  setupRow(m_Shadows, 1);
  setupRow(m_SSAO, 2);
  setupRow(m_Clouds, 3);
  setupRow(m_DayCycleSpeed, 4);

  // Back button at bottom
  float backWidth = 200.0f;
  float backHeight = 40.0f;
  m_BackButton.x = (screenWidth - backWidth) / 2.0f;
  m_BackButton.y = screenHeight * 0.85f;
  m_BackButton.width = backWidth;
  m_BackButton.height = backHeight;
}

void SettingsScreen::Update(float mouseX, float mouseY) {
  // Helper to check button hover
  auto checkHover = [mouseX, mouseY](SettingsButton &btn) {
    btn.hovered = mouseX >= btn.x && mouseX <= btn.x + btn.width &&
                  mouseY >= btn.y && mouseY <= btn.y + btn.height;
  };

  // Check all setting row buttons
  checkHover(m_RenderDistance.decreaseBtn);
  checkHover(m_RenderDistance.increaseBtn);
  checkHover(m_Shadows.decreaseBtn);
  checkHover(m_Shadows.increaseBtn);
  checkHover(m_SSAO.decreaseBtn);
  checkHover(m_SSAO.increaseBtn);
  checkHover(m_Clouds.decreaseBtn);
  checkHover(m_Clouds.increaseBtn);
  checkHover(m_DayCycleSpeed.decreaseBtn);
  checkHover(m_DayCycleSpeed.increaseBtn);

  // Back button
  checkHover(m_BackButton);
}

bool SettingsScreen::OnClick(float mouseX, float mouseY) {
  // Helper to check and handle button click
  auto handleClick = [mouseX, mouseY](SettingsButton &btn) {
    if (btn.hovered) {
      return true;
    }
    return false;
  };

  // Helper to adjust value
  auto adjustValue = [](SettingRow &row, float delta) {
    row.value = glm::clamp(row.value + delta, row.minVal, row.maxVal);
  };

  // Check render distance
  if (handleClick(m_RenderDistance.decreaseBtn)) {
    adjustValue(m_RenderDistance, -m_RenderDistance.step);
    SyncToConfig();
    return true;
  }
  if (handleClick(m_RenderDistance.increaseBtn)) {
    adjustValue(m_RenderDistance, m_RenderDistance.step);
    SyncToConfig();
    return true;
  }

  // Check shadows
  if (handleClick(m_Shadows.decreaseBtn)) {
    adjustValue(m_Shadows, -m_Shadows.step);
    SyncToConfig();
    return true;
  }
  if (handleClick(m_Shadows.increaseBtn)) {
    adjustValue(m_Shadows, m_Shadows.step);
    SyncToConfig();
    return true;
  }

  // Check SSAO
  if (handleClick(m_SSAO.decreaseBtn)) {
    adjustValue(m_SSAO, -m_SSAO.step);
    SyncToConfig();
    return true;
  }
  if (handleClick(m_SSAO.increaseBtn)) {
    adjustValue(m_SSAO, m_SSAO.step);
    SyncToConfig();
    return true;
  }

  // Check clouds
  if (handleClick(m_Clouds.decreaseBtn)) {
    adjustValue(m_Clouds, -m_Clouds.step);
    SyncToConfig();
    return true;
  }
  if (handleClick(m_Clouds.increaseBtn)) {
    adjustValue(m_Clouds, m_Clouds.step);
    SyncToConfig();
    return true;
  }

  // Check day cycle speed
  if (handleClick(m_DayCycleSpeed.decreaseBtn)) {
    adjustValue(m_DayCycleSpeed, -m_DayCycleSpeed.step);
    SyncToConfig();
    return true;
  }
  if (handleClick(m_DayCycleSpeed.increaseBtn)) {
    adjustValue(m_DayCycleSpeed, m_DayCycleSpeed.step);
    SyncToConfig();
    return true;
  }

  // Check back button
  if (handleClick(m_BackButton) && m_OnBack) {
    m_OnBack();
    return true;
  }

  return false;
}

void SettingsScreen::DrawSettingRow(int screenWidth, const SettingRow &row,
                                    const std::string &valueText) {
  (void)screenWidth; // Unused for now
  (void)valueText;   // No text rendering yet

  // Draw decrease button (-)
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &decTex =
        row.decreaseBtn.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (decTex.IsValid()) {
      m_Renderer.DrawTexture(decTex, row.decreaseBtn.x, row.decreaseBtn.y,
                             row.decreaseBtn.width, row.decreaseBtn.height);
    }
    // Draw "-" symbol placeholder (dark rectangle)
    float symWidth = 16.0f;
    float symHeight = 4.0f;
    float symX = row.decreaseBtn.x + (row.decreaseBtn.width - symWidth) / 2.0f;
    float symY =
        row.decreaseBtn.y + (row.decreaseBtn.height - symHeight) / 2.0f;
    m_Renderer.DrawRect(symX, symY, symWidth, symHeight,
                        glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
  }

  // Draw progress bar background
  float barHeight = row.decreaseBtn.height - 8.0f;
  float barY = row.y + 4.0f;
  m_Renderer.DrawRect(row.barX, barY, row.barWidth, barHeight,
                      glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));

  // Draw progress bar fill
  float progress = (row.value - row.minVal) / (row.maxVal - row.minVal);
  float fillWidth = row.barWidth * progress;
  m_Renderer.DrawRect(row.barX, barY, fillWidth, barHeight,
                      glm::vec4(0.2f, 0.5f, 0.2f, 0.9f));

  // Draw increase button (+)
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &incTex =
        row.increaseBtn.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (incTex.IsValid()) {
      m_Renderer.DrawTexture(incTex, row.increaseBtn.x, row.increaseBtn.y,
                             row.increaseBtn.width, row.increaseBtn.height);
    }
    // Draw "+" symbol placeholder (dark cross)
    float symSize = 16.0f;
    float symThick = 4.0f;
    float centerBtnX = row.increaseBtn.x + row.increaseBtn.width / 2.0f;
    float centerBtnY = row.increaseBtn.y + row.increaseBtn.height / 2.0f;
    // Horizontal bar
    m_Renderer.DrawRect(centerBtnX - symSize / 2.0f,
                        centerBtnY - symThick / 2.0f, symSize, symThick,
                        glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
    // Vertical bar
    m_Renderer.DrawRect(centerBtnX - symThick / 2.0f,
                        centerBtnY - symSize / 2.0f, symThick, symSize,
                        glm::vec4(0.2f, 0.2f, 0.2f, 0.9f));
  }
}

void SettingsScreen::Render(int screenWidth, int screenHeight) {
  UpdateLayout(screenWidth, screenHeight);

  m_Renderer.SetScreenSize(screenWidth, screenHeight);
  m_Renderer.Begin();

  // Draw tiled background
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

  // Draw title "Options"
  if (m_TitleOptionsText && m_TitleOptionsText->IsValid()) {
    float texW = static_cast<float>(m_TitleOptionsText->GetWidth());
    float texH = static_cast<float>(m_TitleOptionsText->GetHeight());
    // Use a fixed target width for readability
    float targetWidth = 500.0f;
    float scale = targetWidth / texW;
    float titleWidth = texW * scale;
    float titleHeight = texH * scale;
    float titleX = (screenWidth - titleWidth) / 2.0f;
    float titleY = screenHeight * 0.06f;
    m_Renderer.DrawTexture(*m_TitleOptionsText, titleX, titleY, titleWidth,
                           titleHeight);
  }

  // Draw setting rows
  DrawSettingRow(screenWidth, m_RenderDistance, "");
  DrawSettingRow(screenWidth, m_Shadows, "");
  DrawSettingRow(screenWidth, m_SSAO, "");
  DrawSettingRow(screenWidth, m_Clouds, "");
  DrawSettingRow(screenWidth, m_DayCycleSpeed, "");

  // Draw row labels with text textures
  auto drawLabelText = [&](const SettingRow &row, Core::Texture *tex,
                           float targetWidth) {
    if (tex && tex->IsValid()) {
      float texW = static_cast<float>(tex->GetWidth());
      float texH = static_cast<float>(tex->GetHeight());
      float scale = targetWidth / texW;
      float labelWidth = texW * scale;
      float labelHeight = texH * scale;
      float labelX = row.decreaseBtn.x - labelWidth - 20.0f;
      float labelY = row.y + (row.decreaseBtn.height - labelHeight) / 2.0f;
      m_Renderer.DrawTexture(*tex, labelX, labelY, labelWidth, labelHeight);
    }
  };
  drawLabelText(m_RenderDistance, m_RenderDistanceText.get(), 180.0f);
  drawLabelText(m_Shadows, m_ShadowsText.get(), 120.0f);
  drawLabelText(m_SSAO, m_SSAOText.get(), 80.0f);
  drawLabelText(m_Clouds, m_CloudsText.get(), 100.0f);
  drawLabelText(m_DayCycleSpeed, m_DayCycleText.get(), 140.0f);

  // Draw back button
  if (m_ButtonTex && m_ButtonHoverTex) {
    Core::Texture &backTex =
        m_BackButton.hovered ? *m_ButtonHoverTex : *m_ButtonTex;
    if (backTex.IsValid()) {
      m_Renderer.DrawTexture(backTex, m_BackButton.x, m_BackButton.y,
                             m_BackButton.width, m_BackButton.height);
    }
    // "Done" text
    if (m_DoneText && m_DoneText->IsValid()) {
      float texW = static_cast<float>(m_DoneText->GetWidth());
      float texH = static_cast<float>(m_DoneText->GetHeight());
      // Use a fixed target width for readability
      float targetWidth = 120.0f;
      float scale = targetWidth / texW;
      float textWidth = texW * scale;
      float textHeight = texH * scale;
      float textX = m_BackButton.x + (m_BackButton.width - textWidth) / 2.0f;
      float textY = m_BackButton.y + (m_BackButton.height - textHeight) / 2.0f;
      m_Renderer.DrawTexture(*m_DoneText, textX, textY, textWidth, textHeight);
    }
  }

  m_Renderer.End();
}

} // namespace UI
