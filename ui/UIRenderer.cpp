#include "ui/UIRenderer.h"
#include "core/Logger.h"
#include "core/Shader.h"
#include "core/Texture.h"


#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

namespace UI {

UIRenderer::UIRenderer() {
  // Load shaders
  m_TexturedShader = std::make_unique<Core::Shader>(
      "assets/shaders/ui_textured.vert", "assets/shaders/ui_textured.frag");

  m_ColorShader = std::make_unique<Core::Shader>("assets/shaders/ui.vert",
                                                 "assets/shaders/ui.frag");

  SetupBuffers();
  LOG_INFO("UIRenderer initialized");
}

UIRenderer::~UIRenderer() { CleanupBuffers(); }

void UIRenderer::SetupBuffers() {
  // Create VAO for dynamic quad rendering
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);

  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Allocate buffer for a single quad (6 vertices, 4 floats each: x, y, u, v)
  glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr,
               GL_DYNAMIC_DRAW);

  // Position attribute (location 0)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // TexCoord attribute (location 1)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void UIRenderer::CleanupBuffers() {
  if (m_VBO) {
    glDeleteBuffers(1, &m_VBO);
    m_VBO = 0;
  }
  if (m_VAO) {
    glDeleteVertexArrays(1, &m_VAO);
    m_VAO = 0;
  }
}

void UIRenderer::SetScreenSize(int width, int height) {
  m_ScreenWidth = width;
  m_ScreenHeight = height;

  // Create orthographic projection (origin at top-left, Y down)
  m_Projection = glm::ortho(0.0f, static_cast<float>(width),
                            static_cast<float>(height), 0.0f, -1.0f, 1.0f);
}

void UIRenderer::Begin() {
  // Disable depth testing for UI
  glDisable(GL_DEPTH_TEST);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Disable backface culling (UI is 2D)
  glDisable(GL_CULL_FACE);
}

void UIRenderer::End() {
  // Restore 3D rendering state
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
}

void UIRenderer::UpdateQuad(float x, float y, float width, float height,
                            float u0, float v0, float u1, float v1) {
  // 6 vertices for 2 triangles (x, y, u, v)
  float vertices[] = {// First triangle (top-left, top-right, bottom-left)
                      x, y, u0, v0, x + width, y, u1, v0, x, y + height, u0, v1,

                      // Second triangle (top-right, bottom-right, bottom-left)
                      x + width, y, u1, v0, x + width, y + height, u1, v1, x,
                      y + height, u0, v1};

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

void UIRenderer::DrawTexture(Core::Texture &texture, float x, float y,
                             float width, float height, const glm::vec4 &tint) {
  DrawTextureUV(texture, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, tint);
}

void UIRenderer::DrawTextureUV(Core::Texture &texture, float x, float y,
                               float width, float height, float u0, float v0,
                               float u1, float v1, const glm::vec4 &tint) {
  if (!m_TexturedShader || !m_TexturedShader->IsValid())
    return;

  UpdateQuad(x, y, width, height, u0, v0, u1, v1);

  m_TexturedShader->Bind();
  m_TexturedShader->SetMat4("u_Projection", m_Projection);
  m_TexturedShader->SetVec4("u_Tint", tint);
  m_TexturedShader->SetInt("u_Texture", 0);

  texture.Bind(0);

  glBindVertexArray(m_VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  m_TexturedShader->Unbind();
}

void UIRenderer::DrawRect(float x, float y, float width, float height,
                          const glm::vec4 &color) {
  if (!m_ColorShader || !m_ColorShader->IsValid())
    return;

  // Convert to NDC for the simple UI shader
  float ndcX1 = (x / m_ScreenWidth) * 2.0f - 1.0f;
  float ndcY1 = 1.0f - (y / m_ScreenHeight) * 2.0f;
  float ndcX2 = ((x + width) / m_ScreenWidth) * 2.0f - 1.0f;
  float ndcY2 = 1.0f - ((y + height) / m_ScreenHeight) * 2.0f;

  float vertices[] = {ndcX1, ndcY1, 0.0f, 0.0f, ndcX2, ndcY1, 0.0f, 0.0f,
                      ndcX1, ndcY2, 0.0f, 0.0f, ndcX2, ndcY1, 0.0f, 0.0f,
                      ndcX2, ndcY2, 0.0f, 0.0f, ndcX1, ndcY2, 0.0f, 0.0f};

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  m_ColorShader->Bind();
  m_ColorShader->SetVec4("u_Color", color);

  glBindVertexArray(m_VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  m_ColorShader->Unbind();
}

void UIRenderer::DrawTiled(Core::Texture &texture, float x, float y,
                           float width, float height, float tileWidth,
                           float tileHeight) {
  // Draw tiles to cover the entire area
  for (float ty = y; ty < y + height; ty += tileHeight) {
    for (float tx = x; tx < x + width; tx += tileWidth) {
      float tw = std::min(tileWidth, (x + width) - tx);
      float th = std::min(tileHeight, (y + height) - ty);

      // Calculate UV coords for partial tiles at edges
      float u1 = tw / tileWidth;
      float v1 = th / tileHeight;

      DrawTextureUV(texture, tx, ty, tw, th, 0.0f, 0.0f, u1, v1);
    }
  }
}

} // namespace UI
