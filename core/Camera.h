#pragma once

#include "Frustum.h"
#include <glm/glm.hpp>

namespace Core {

class Camera {
public:
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
         float yaw = -90.0f, float pitch = 0.0f);

  // Get matrices
  [[nodiscard]] glm::mat4 GetViewMatrix() const;
  [[nodiscard]] glm::mat4 GetProjectionMatrix(float aspectRatio) const;
  [[nodiscard]] glm::mat4 GetViewProjectionMatrix(float aspectRatio) const;

  // Frustum culling
  void UpdateFrustum(float aspectRatio);
  [[nodiscard]] const Frustum& GetFrustum() const { return m_Frustum; }

  // Movement
  void ProcessKeyboard(float deltaTime, bool forward, bool backward, 
                       bool left, bool right, bool up, bool down);
  void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

  // Getters
  [[nodiscard]] glm::vec3 GetPosition() const { return m_Position; }
  [[nodiscard]] glm::vec3 GetFront() const { return m_Front; }
  [[nodiscard]] float GetFOV() const { return m_FOV; }

  // Setters
  void SetPosition(const glm::vec3 &position) { m_Position = position; }
  void SetSpeed(float speed) { m_Speed = speed; }
  void SetSensitivity(float sensitivity) { m_Sensitivity = sensitivity; }
  void SetFOV(float fov) { m_FOV = fov; }

private:
  void UpdateCameraVectors();

  // Camera attributes
  glm::vec3 m_Position;
  glm::vec3 m_Front;
  glm::vec3 m_Up;
  glm::vec3 m_Right;
  glm::vec3 m_WorldUp;

  // Euler angles
  float m_Yaw;
  float m_Pitch;

  // Camera options
  float m_Speed = 5.0f;
  float m_Sensitivity = 0.1f;
  float m_FOV = 45.0f;
  float m_NearPlane = 0.1f;
  float m_FarPlane = 500.0f;  // Increased for larger view distances

  // Cached frustum for culling
  Frustum m_Frustum;
};

} // namespace Core
