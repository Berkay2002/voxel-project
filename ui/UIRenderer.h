#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Core {
class Shader;
class Texture;
} // namespace Core

namespace UI {

/// Renders 2D UI elements (textured quads, colored rectangles)
class UIRenderer {
public:
  UIRenderer();
  ~UIRenderer();

  // Non-copyable
  UIRenderer(const UIRenderer &) = delete;
  UIRenderer &operator=(const UIRenderer &) = delete;

  /// Set the screen size for orthographic projection
  void SetScreenSize(int width, int height);

  /// Begin a UI rendering batch (sets up state)
  void Begin();

  /// End UI rendering batch (restores state)
  void End();

  /// Draw a textured quad
  /// @param texture The texture to draw
  /// @param x Screen X position (pixels, left edge)
  /// @param y Screen Y position (pixels, top edge)
  /// @param width Width in pixels
  /// @param height Height in pixels
  /// @param tint Optional color tint (default white)
  void DrawTexture(Core::Texture &texture, float x, float y, float width,
                   float height, const glm::vec4 &tint = glm::vec4(1.0f));

  /// Draw a textured quad with UV coordinates for sprite sheets
  void DrawTextureUV(Core::Texture &texture, float x, float y, float width,
                     float height, float u0, float v0, float u1, float v1,
                     const glm::vec4 &tint = glm::vec4(1.0f));

  /// Draw a solid colored rectangle
  void DrawRect(float x, float y, float width, float height,
                const glm::vec4 &color);

  /// Draw a tiled texture (for backgrounds)
  void DrawTiled(Core::Texture &texture, float x, float y, float width,
                 float height, float tileWidth, float tileHeight);

private:
  void SetupBuffers();
  void CleanupBuffers();
  void UpdateQuad(float x, float y, float width, float height, float u0,
                  float v0, float u1, float v1);

  std::unique_ptr<Core::Shader> m_TexturedShader;
  std::unique_ptr<Core::Shader> m_ColorShader;

  unsigned int m_VAO = 0;
  unsigned int m_VBO = 0;

  glm::mat4 m_Projection = glm::mat4(1.0f);
  int m_ScreenWidth = 800;
  int m_ScreenHeight = 600;
};

} // namespace UI
