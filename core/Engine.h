#pragma once

#include <memory>

namespace Core {

class Window;
class Shader;
class Texture;
class Camera;

} // namespace Core

namespace Voxel {
class ChunkManager;
}

namespace Core {

class Engine {
public:
  Engine();
  ~Engine();

  // Non-copyable, non-movable
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;
  Engine(Engine &&) = delete;
  Engine &operator=(Engine &&) = delete;

  void Run();

private:
  void Update(float deltaTime);
  void Render();
  void SetupWorld();
  void ProcessInput(float deltaTime);

  std::unique_ptr<Window> m_Window;
  
  // Rendering resources
  std::unique_ptr<Shader> m_Shader;
  std::unique_ptr<Texture> m_Texture;
  std::unique_ptr<Camera> m_Camera;

  // World system
  std::unique_ptr<Voxel::ChunkManager> m_ChunkManager;

  // Input state
  float m_LastX = 400.0f;
  float m_LastY = 300.0f;
  bool m_FirstMouse = true;
  bool m_CursorCaptured = false;
};

} // namespace Core
