#include "ShadowMap.h"
#include "../Logger.h"
#include <glad/gl.h>
#include <string>

namespace Core {

ShadowMap::~ShadowMap() {
  if (m_DepthTexture != 0) {
    glDeleteTextures(1, &m_DepthTexture);
  }
  if (m_FBO != 0) {
    glDeleteFramebuffers(1, &m_FBO);
  }
}

bool ShadowMap::Create(int width, int height) {
  m_Width = width;
  m_Height = height;

  // Create framebuffer
  glGenFramebuffers(1, &m_FBO);

  // Create depth texture
  glGenTextures(1, &m_DepthTexture);
  glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  // Texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Clamp to border with white (1.0) - outside shadow map = not in shadow
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  // Enable hardware shadow comparison (for sampler2DShadow)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_REF_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

  // Attach depth texture to FBO
  glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         m_DepthTexture, 0);

  // No color buffer for shadow map
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // Check framebuffer completeness
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    LOG_ERROR("ShadowMap FBO not complete! Status: " + std::to_string(status));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  LOG_INFO("Created shadow map: " + std::to_string(width) + "x" +
           std::to_string(height));
  return true;
}

void ShadowMap::Bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_FBO); }

void ShadowMap::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void ShadowMap::BindTexture(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
}

void ShadowMap::UnbindTexture(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Core
