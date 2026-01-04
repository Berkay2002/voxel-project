#pragma once

#include "world/VoxelRaycast.h"
#include <memory>
#include <glm/glm.hpp>

namespace Core {

class Window;
class Shader;
class Texture;
class TextureArray;
class Camera;
class BlockOutline;
class ShadowMap;

} // namespace Core

namespace Voxel {
class ChunkManager;
class SkyRenderer;
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

  // Mouse button callback (called from GLFW callback)
  void OnMouseButton(int button, int action);

private:
  void Update(float deltaTime);
  void Render();
  void RenderShadowPass();  // Shadow map depth pass
  void SetupWorld();
  void ProcessInput(float deltaTime);
  void UpdateTargetedBlock();

  std::unique_ptr<Window> m_Window;
  
  // Rendering resources
  std::unique_ptr<Shader> m_Shader;           // Opaque geometry shader (lit)
  std::unique_ptr<Shader> m_WaterShader;      // Water shader (transparent)
  std::unique_ptr<Shader> m_UIShader;         // UI shader (crosshair, etc.)
  std::unique_ptr<Shader> m_OutlineShader;    // Block outline shader
  std::unique_ptr<Shader> m_ShadowShader;     // Shadow depth pass shader
  // Note: Textures are managed by TextureRegistry singleton
  std::unique_ptr<Camera> m_Camera;

  // Shadow mapping
  std::unique_ptr<ShadowMap> m_ShadowMap;
  glm::mat4 m_LightSpaceMatrix = glm::mat4(1.0f);

  // World system
  std::unique_ptr<Voxel::ChunkManager> m_ChunkManager;
  std::unique_ptr<Voxel::SkyRenderer> m_SkyRenderer;

  // Block interaction (raycasting)
  Voxel::RaycastResult m_TargetedBlock;       // Currently targeted block
  Voxel::BlockID m_SelectedBlockType = 3;     // Block type to place (Stone by default)

  // Block outline rendering
  std::unique_ptr<BlockOutline> m_BlockOutline;

  // UI rendering (crosshair)
  unsigned int m_CrosshairVAO = 0;
  unsigned int m_CrosshairVBO = 0;
  void SetupCrosshair();
  void RenderCrosshair();
  void CleanupCrosshair();

  // Input state
  float m_LastX = 400.0f;
  float m_LastY = 300.0f;
  bool m_FirstMouse = true;
  bool m_CursorCaptured = false;
};

} // namespace Core

