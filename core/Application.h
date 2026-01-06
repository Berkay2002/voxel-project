#pragma once

#include "core/GameState.h"
#include "world/VoxelRaycast.h"
#include <glm/glm.hpp>
#include <memory>

namespace Core {

class Window;
class Shader;
class Texture;
class TextureArray;
class Camera;
class SelectionRenderer;
class ShadowMap;
class SSAO;
class SkySystem;

} // namespace Core

namespace Voxel {
class ChunkManager;
} // namespace Voxel

namespace UI {
class UIRenderer;
class TitleScreen;
class LoadingScreen;
class SettingsScreen;
} // namespace UI

namespace Core {

class Application {
public:
  Application();
  ~Application();

  // Non-copyable, non-movable
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application &&) = delete;

  void Run();

  // Mouse button callback (called from GLFW callback)
  void OnMouseButton(int button, int action);

private:
  // State-based update/render methods
  void UpdateTitleScreen(float deltaTime);
  void UpdateSettingsScreen(float deltaTime);
  void UpdateLoadingScreen(float deltaTime);
  void RenderTitleScreen();
  void RenderSettingsScreen();
  void RenderLoadingScreen();
  void TransitionToState(GameState newState);

  // Gameplay methods
  void Update(float deltaTime);
  void Render();
  void RenderShadowPass(); // Shadow map depth pass
  void RenderSSAOPass();   // SSAO depth + calculation passes
  void SetupWorld();
  void ProcessInput(float deltaTime);
  void UpdateTargetedBlock();

  // Game state
  GameState m_CurrentState = GameState::TITLE_SCREEN;
  float m_LoadingProgress = 0.0f;
  bool m_bWorldSetupStarted = false;

  std::unique_ptr<Window> m_Window;

  // UI system
  std::unique_ptr<UI::UIRenderer> m_UIRenderer;
  std::unique_ptr<UI::TitleScreen> m_TitleScreen;
  std::unique_ptr<UI::LoadingScreen> m_LoadingScreen;
  std::unique_ptr<UI::SettingsScreen> m_SettingsScreen;

  // Rendering resources
  std::unique_ptr<Shader> m_Shader;        // Opaque geometry shader (lit)
  std::unique_ptr<Shader> m_WaterShader;   // Water shader (transparent)
  std::unique_ptr<Shader> m_UIShader;      // UI shader (crosshair, etc.)
  std::unique_ptr<Shader> m_OutlineShader; // Block outline shader
  std::unique_ptr<Shader> m_ShadowShader;  // Shadow depth pass shader
  // Note: Textures are managed by TextureRegistry singleton
  std::unique_ptr<Camera> m_Camera;

  // Shadow mapping
  std::unique_ptr<ShadowMap> m_ShadowMap;
  std::unique_ptr<SSAO> m_SSAO;
  glm::mat4 m_LightSpaceMatrix = glm::mat4(1.0f);

  // World system
  std::unique_ptr<Voxel::ChunkManager> m_ChunkManager;
  std::unique_ptr<SkySystem> m_SkySystem;

  // Block interaction (raycasting)
  Voxel::RaycastResult m_TargetedBlock; // Currently targeted block
  Voxel::BlockID m_SelectedBlockType =
      3; // Block type to place (Stone by default)

  // Block selection rendering (wireframe outline)
  std::unique_ptr<SelectionRenderer> m_SelectionRenderer;

  // UI rendering (crosshair)
  unsigned int m_CrosshairVAO = 0;
  unsigned int m_CrosshairVBO = 0;
  void SetupCrosshair();
  void RenderCrosshair();
  void CleanupCrosshair();

  // Input state
  float m_LastX = 400.0f;
  float m_LastY = 300.0f;
  bool m_bFirstMouse = true;
  bool m_bCursorCaptured = false;
};

} // namespace Core
