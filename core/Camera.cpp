#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace Core {

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : m_Position(position),
      m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      m_Yaw(yaw),
      m_Pitch(pitch) {
  UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
  return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
  return glm::perspective(glm::radians(m_FOV), aspectRatio, m_NearPlane, m_FarPlane);
}

glm::mat4 Camera::GetViewProjectionMatrix(float aspectRatio) const {
  return GetProjectionMatrix(aspectRatio) * GetViewMatrix();
}

void Camera::ProcessKeyboard(float deltaTime, bool forward, bool backward,
                              bool left, bool right, bool up, bool down, bool sprint) {
  // Sprint multiplier (3x speed when holding CTRL)
  float speedMultiplier = sprint ? 3.0f : 1.0f;
  float velocity = m_Speed * speedMultiplier * deltaTime;

  if (forward)
    m_Position += m_Front * velocity;
  if (backward)
    m_Position -= m_Front * velocity;
  if (left)
    m_Position -= m_Right * velocity;
  if (right)
    m_Position += m_Right * velocity;
  if (up)
    m_Position += m_WorldUp * velocity;
  if (down)
    m_Position -= m_WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
  xOffset *= m_Sensitivity;
  yOffset *= m_Sensitivity;

  m_Yaw += xOffset;
  m_Pitch += yOffset;

  // Constrain pitch to avoid gimbal lock
  if (constrainPitch) {
    if (m_Pitch > 89.0f)
      m_Pitch = 89.0f;
    if (m_Pitch < -89.0f)
      m_Pitch = -89.0f;
  }

  UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() {
  // Calculate the new front vector from yaw and pitch
  glm::vec3 front;
  front.x = std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
  front.y = std::sin(glm::radians(m_Pitch));
  front.z = std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
  m_Front = glm::normalize(front);

  // Recalculate right and up vectors
  m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
  m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

void Camera::UpdateFrustum(float aspectRatio) {
  glm::mat4 viewProj = GetViewProjectionMatrix(aspectRatio);
  m_Frustum.ExtractPlanes(viewProj);
}

} // namespace Core
