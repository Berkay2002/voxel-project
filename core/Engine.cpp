#include "core/Engine.h"
#include "core/BlockOutline.h"
#include "core/Camera.h"
#include "core/Logger.h"
#include "core/SSAO.h"
#include "core/Shader.h"
#include "core/ShadowMap.h"
#include "core/TextureArray.h"
#include "core/TextureRegistry.h"
#include "core/Window.h"

// UI system
#include "ui/LoadingScreen.h"
#include "ui/SettingsScreen.h"
#include "ui/TitleScreen.h"
#include "ui/UIRenderer.h"

// Voxel system
#include "world/BlockRegistry.h"
#include "world/ChunkManager.h"
#include "world/SkyRenderer.h"
#include "world/SpaghettiCaveCarver.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace Core {

// Static pointer for GLFW callback access (can't use glfwSetWindowUserPointer,
// it's already used by Window class for resize callback)
static Engine *s_Instance = nullptr;

Engine::Engine() {
  LOG_INFO("Initializing Engine...");

  // Create window (1280x720 for better menu visibility)
  m_Window = std::make_unique<Window>(1280, 720, "VoxelCraft");

  // Load OpenGL functions via GLAD
  if (!gladLoadGL(glfwGetProcAddress)) {
    LOG_ERROR("Failed to initialize GLAD");
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // Log OpenGL version
  const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  const char *renderer =
      reinterpret_cast<const char *>(glGetString(GL_RENDERER));
  LOG_INFO(std::string("OpenGL Version: ") + (version ? version : "unknown"));
  LOG_INFO(std::string("Renderer: ") + (renderer ? renderer : "unknown"));

  // Set viewport resize callback (captures this for SSAO resize)
  m_Window->SetResizeCallback([this](int width, int height) {
    glViewport(0, 0, width, height);
    // Resize SSAO render targets
    if (m_SSAO) {
      m_SSAO->Resize(width, height);
    }
    LOG_DEBUG("Viewport resized: " + std::to_string(width) + "x" +
              std::to_string(height));
  });

  // Initial viewport
  glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

  // Enable depth testing for 3D
  glEnable(GL_DEPTH_TEST);

  // Enable backface culling for performance
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Create camera - position it to view the terrain (lower for cave visibility)
  m_Camera = std::make_unique<Camera>(glm::vec3(8.0f, 60.0f, 40.0f));

  // Setup mouse button callback
  s_Instance = this;
  glfwSetMouseButtonCallback(
      m_Window->GetHandle(),
      [](GLFWwindow * /*window*/, int button, int action, int /*mods*/) {
        if (s_Instance) {
          s_Instance->OnMouseButton(button, action);
        }
      });

  // ====== SETUP UI SYSTEM ======
  m_UIRenderer = std::make_unique<UI::UIRenderer>();

  // Setup Title Screen
  m_TitleScreen = std::make_unique<UI::TitleScreen>(*m_UIRenderer);
  m_TitleScreen->SetOnPlay([this]() { TransitionToState(GameState::LOADING); });
  m_TitleScreen->SetOnOptions(
      [this]() { TransitionToState(GameState::SETTINGS); });
  m_TitleScreen->SetOnQuit(
      [this]() { glfwSetWindowShouldClose(m_Window->GetHandle(), true); });

  // Setup Loading Screen
  m_LoadingScreen = std::make_unique<UI::LoadingScreen>(*m_UIRenderer);

  // Setup Settings Screen
  m_SettingsScreen = std::make_unique<UI::SettingsScreen>(*m_UIRenderer);
  m_SettingsScreen->SetWindow(m_Window.get());
  m_SettingsScreen->SetOnBack(
      [this]() { TransitionToState(GameState::TITLE_SCREEN); });

  // Start in title screen state (world setup deferred to loading state)
  m_CurrentState = GameState::TITLE_SCREEN;

  // Show cursor for menu navigation
  glfwSetInputMode(m_Window->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  LOG_INFO("Engine initialized - showing title screen");
}

Engine::~Engine() {
  CleanupCrosshair();
  s_Instance = nullptr; // Clear static pointer
  LOG_INFO("Engine shutting down...");
}

void Engine::SetupWorld() {
  // Create lit shader with lighting support (for opaque geometry)
  m_Shader = std::make_unique<Shader>("assets/shaders/lit.vert",
                                      "assets/shaders/lit.frag");

  if (!m_Shader->IsValid()) {
    LOG_ERROR("Failed to create lit shader for world");
    return;
  }

  // Create water shader (for transparent water)
  m_WaterShader = std::make_unique<Shader>("assets/shaders/water.vert",
                                           "assets/shaders/water.frag");

  if (!m_WaterShader->IsValid()) {
    LOG_ERROR("Failed to create water shader");
    return;
  }

  // Create UI shader (for crosshair and other 2D elements)
  m_UIShader = std::make_unique<Shader>("assets/shaders/ui.vert",
                                        "assets/shaders/ui.frag");

  if (!m_UIShader->IsValid()) {
    LOG_ERROR("Failed to create UI shader");
    return;
  }

  // Create outline shader (for block highlighting)
  m_OutlineShader = std::make_unique<Shader>("assets/shaders/outline.vert",
                                             "assets/shaders/outline.frag");

  if (!m_OutlineShader->IsValid()) {
    LOG_ERROR("Failed to create outline shader");
    return;
  }

  // Create shadow shader (depth-only pass for shadow mapping)
  m_ShadowShader = std::make_unique<Shader>("assets/shaders/shadow.vert",
                                            "assets/shaders/shadow.frag");

  if (!m_ShadowShader->IsValid()) {
    LOG_ERROR("Failed to create shadow shader");
    return;
  }

  // Create shadow map (depth-only FBO)
  m_ShadowMap = std::make_unique<ShadowMap>();
  if (!m_ShadowMap->Create(Voxel::Config::SHADOW_MAP_RESOLUTION,
                           Voxel::Config::SHADOW_MAP_RESOLUTION)) {
    LOG_ERROR("Failed to create shadow map");
    // Non-fatal: continue without shadows
    m_ShadowMap.reset();
  } else {
    LOG_INFO("Shadow map created: " +
             std::to_string(Voxel::Config::SHADOW_MAP_RESOLUTION) + "x" +
             std::to_string(Voxel::Config::SHADOW_MAP_RESOLUTION));
  }

  // Create SSAO system (half-resolution for performance)
  m_SSAO = std::make_unique<SSAO>();
  if (!m_SSAO->Setup(m_Window->GetWidth(), m_Window->GetHeight())) {
    LOG_WARN("SSAO setup failed, disabling");
    m_SSAO.reset();
  } else {
    LOG_INFO("SSAO initialized (O key to toggle)");
  }

  // Setup crosshair
  SetupCrosshair();

  // Setup block outline
  m_BlockOutline = std::make_unique<BlockOutline>();
  m_BlockOutline->Setup();

  // =========================================================================
  // PHASE 11: Data-driven texture and block loading via registries
  // =========================================================================

  // Define textures to load (order determines layer indices)
  // This list should include all textures referenced in blocks.json
  std::vector<std::string> textureList = {
      "grass_block_top",  // Layer 0
      "dirt",             // Layer 1
      "grass_block_side", // Layer 2
      "stone",            // Layer 3
      "blue_ice",    // Layer 4 (water placeholder - water_still is animated
                     // spritesheet)
      "bedrock",     // Layer 5
      "sand",        // Layer 6
      "gravel",      // Layer 7
      "cobblestone", // Layer 8
      "oak_log_top", // Layer 9
      "oak_log",     // Layer 10
      "oak_planks",  // Layer 11
      "oak_leaves",  // Layer 12
      "coal_ore",    // Layer 13
      "iron_ore",    // Layer 14
      "gold_ore",    // Layer 15
      "diamond_ore", // Layer 16
      "copper_ore",  // Layer 17
      "emerald_ore"  // Layer 18
  };

  // Load textures via TextureRegistry
  TextureRegistry &texRegistry = TextureRegistry::Instance();
  if (!texRegistry.LoadTextures(textureList, "assets/textures/blocks/")) {
    LOG_ERROR("Failed to load textures via TextureRegistry");
    return;
  }

  // Load block definitions via BlockRegistry
  Voxel::BlockRegistry &blockRegistry = Voxel::BlockRegistry::Instance();
  if (!blockRegistry.LoadFromFile("assets/config/blocks.json", texRegistry)) {
    LOG_ERROR("Failed to load block definitions via BlockRegistry");
    return;
  }

  // Set lit shader uniforms
  m_Shader->Bind();
  m_Shader->SetInt("u_TextureArray", 0); // Texture array in slot 0
  // Sun direction: slightly angled from above-right
  m_Shader->SetVec3("u_LightDir", glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));
  // Ambient strength: prevents pure black shadows
  m_Shader->SetFloat("u_AmbientStrength", 0.35f);
  // Fog settings (color matches sky background)
  m_Shader->SetVec3("u_FogColor", glm::vec3(0.5f, 0.7f, 1.0f));
  m_Shader->SetFloat("u_FogStart", Voxel::Config::FOG_START);
  m_Shader->SetFloat("u_FogEnd", Voxel::Config::FOG_END);
  m_Shader->SetInt("u_ShadowMap", 1);           // Shadow map in slot 1
  m_Shader->SetBool("u_ShadowsEnabled", false); // Will be set per-frame
  m_Shader->Unbind();

  // Set water shader uniforms
  m_WaterShader->Bind();
  m_WaterShader->SetInt("u_TextureArray", 0); // Same texture array
  m_WaterShader->SetVec3("u_LightDir",
                         glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));
  m_WaterShader->SetFloat("u_AmbientStrength", 0.35f);
  m_WaterShader->SetFloat("u_WaterAlpha", 0.7f); // Water transparency
  m_WaterShader->SetFloat("u_Time", 0.0f);       // For optional animation
  // Fog settings (same as lit shader)
  m_WaterShader->SetVec3("u_FogColor", glm::vec3(0.5f, 0.7f, 1.0f));
  m_WaterShader->SetFloat("u_FogStart", Voxel::Config::FOG_START);
  m_WaterShader->SetFloat("u_FogEnd", Voxel::Config::FOG_END);
  m_WaterShader->Unbind();

  // Create chunk manager
  m_ChunkManager = std::make_unique<Voxel::ChunkManager>();

  // Create and setup sky renderer
  m_SkyRenderer = std::make_unique<Voxel::SkyRenderer>();
  if (!m_SkyRenderer->Setup()) {
    LOG_ERROR("Failed to setup SkyRenderer");
    // Non-fatal: continue without sky
  }

  LOG_INFO("World setup complete with " +
           std::to_string(blockRegistry.GetBlockCount()) + " blocks and " +
           std::to_string(texRegistry.GetTextureCount()) + " textures");
}

void Engine::ProcessInput(float deltaTime) {
  GLFWwindow *window = m_Window->GetHandle();

  // Close window with ESC
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  // Toggle mouse capture with M key
  static bool mKeyWasPressed = false;
  if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
    if (!mKeyWasPressed) {
      mKeyWasPressed = true;
      m_CursorCaptured = !m_CursorCaptured;
      if (m_CursorCaptured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_FirstMouse = true;
        LOG_INFO("Mouse captured - use M to release");
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        LOG_INFO("Mouse released - use M to capture");
      }
    }
  } else {
    mKeyWasPressed = false;
  }

  // Toggle weather with K key
  static bool kKeyWasPressed = false;
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    if (!kKeyWasPressed) {
      kKeyWasPressed = true;
      if (m_SkyRenderer) {
        m_SkyRenderer->ToggleWeather();
        if (m_SkyRenderer->IsWeatherEnabled()) {
          LOG_INFO("Weather enabled (K to toggle)");
        } else {
          LOG_INFO("Weather disabled (K to toggle)");
        }
      }
    }
  } else {
    kKeyWasPressed = false;
  }

  // Fast-forward time with J key (hold to advance quickly)
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    if (m_SkyRenderer) {
      // Advance time by 5% per second while held (20 seconds = full day cycle)
      float newTime = m_SkyRenderer->GetTimeOfDay() + deltaTime * 0.05f;
      m_SkyRenderer->SetTimeOfDay(newTime);
    }
  }

  // Rewind time with H key (hold to go back quickly - towards night)
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    if (m_SkyRenderer) {
      // Rewind time by 5% per second while held
      float newTime = m_SkyRenderer->GetTimeOfDay() - deltaTime * 0.05f;
      m_SkyRenderer->SetTimeOfDay(newTime);
    }
  }

  // Toggle SSAO with O key
  static bool oKeyWasPressed = false;
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    if (!oKeyWasPressed) {
      oKeyWasPressed = true;
      if (m_SSAO) {
        m_SSAO->SetEnabled(!m_SSAO->IsEnabled());
        if (m_SSAO->IsEnabled()) {
          LOG_INFO("SSAO enabled (O to toggle)");
        } else {
          LOG_INFO("SSAO disabled - using vertex AO (O to toggle)");
        }
      }
    }
  } else {
    oKeyWasPressed = false;
  }

  // Keyboard input
  bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
  bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
  bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
  bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
  bool up = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
  bool down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  bool sprint = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

  m_Camera->ProcessKeyboard(deltaTime, forward, backward, left, right, up, down,
                            sprint);

  // Mouse input (only when cursor is captured)
  if (m_CursorCaptured) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (m_FirstMouse) {
      m_LastX = static_cast<float>(xpos);
      m_LastY = static_cast<float>(ypos);
      m_FirstMouse = false;
    }

    float xOffset = static_cast<float>(xpos) - m_LastX;
    float yOffset = m_LastY - static_cast<float>(
                                  ypos); // Reversed: y-coords go bottom to top

    m_LastX = static_cast<float>(xpos);
    m_LastY = static_cast<float>(ypos);

    m_Camera->ProcessMouseMovement(xOffset, yOffset);
  }
}

void Engine::Run() {
  LOG_INFO("Starting main loop...");

  float lastFrame = 0.0f;

  while (!m_Window->ShouldClose()) {
    float currentFrame = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwPollEvents();

    // State-based game loop
    switch (m_CurrentState) {
    case GameState::TITLE_SCREEN:
      UpdateTitleScreen(deltaTime);
      RenderTitleScreen();
      break;

    case GameState::SETTINGS:
      UpdateSettingsScreen(deltaTime);
      RenderSettingsScreen();
      break;

    case GameState::LOADING:
      UpdateLoadingScreen(deltaTime);
      RenderLoadingScreen();
      break;

    case GameState::PLAYING:
      Update(deltaTime);
      Render();
      break;
    }

    m_Window->SwapBuffers();
  }

  LOG_INFO("Main loop ended");
}

void Engine::Update(float deltaTime) {
  // Note: glfwPollEvents called in Run() before state switch
  ProcessInput(deltaTime);

  // Update targeted block for interaction (raycast from camera)
  UpdateTargetedBlock();

  // Update sky (time of day, cloud drift)
  if (m_SkyRenderer && m_Camera) {
    m_SkyRenderer->Update(deltaTime, m_Camera->GetPosition());
  }

  // Update chunk loading based on camera position
  if (m_ChunkManager && m_Camera) {
    m_ChunkManager->Update(m_Camera->GetPosition());
    // Process completed async mesh generations (upload to GPU on main thread)
    m_ChunkManager->ProcessPendingMeshes();
  }
}

void Engine::RenderShadowPass() {
  if (!m_ShadowMap || !m_ShadowShader || !m_ChunkManager || !m_Camera) {
    return;
  }

  // Get sun direction from SkyRenderer
  glm::vec3 sunDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
  if (m_SkyRenderer) {
    sunDir = m_SkyRenderer->GetSunDirection();
  }

  // Skip shadow pass if sun is below horizon
  if (sunDir.y < 0.1f) {
    return;
  }

  // Calculate light-space matrix with shadow map stabilization
  glm::vec3 cameraPos = m_Camera->GetPosition();
  float shadowDistance = Voxel::Config::SHADOW_DISTANCE;

  // === SHADOW MAP STABILIZATION ===
  // Snap the shadow frustum center to texel boundaries to prevent shadow
  // swimming when the player moves. This keeps shadows stable in world space.

  // Step 1: Create a stable "light view" looking along sun direction
  // We use a fixed up vector orthogonal to the light direction
  glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
  if (std::abs(sunDir.y) > 0.99f) {
    // Sun is nearly vertical, use different up vector
    lightUp = glm::vec3(0.0f, 0.0f, 1.0f);
  }
  glm::vec3 lightRight = glm::normalize(glm::cross(sunDir, lightUp));
  lightUp = glm::normalize(glm::cross(lightRight, sunDir));

  // Create view matrix components manually to avoid lookAt instabilities
  glm::mat4 lightView = glm::mat4(1.0f);
  lightView[0] = glm::vec4(lightRight, 0.0f);
  lightView[1] = glm::vec4(lightUp, 0.0f);
  lightView[2] = glm::vec4(-sunDir, 0.0f); // Negative because looking along -Z
  lightView = glm::transpose(lightView);   // Rotation part

  // Step 2: Transform camera position to light space
  glm::vec3 cameraPosLightSpace =
      glm::vec3(lightView * glm::vec4(cameraPos, 1.0f));

  // Step 3: Snap to texel grid to prevent shadow swimming
  float texelSize =
      (shadowDistance * 2.0f) / static_cast<float>(m_ShadowMap->GetWidth());
  cameraPosLightSpace.x =
      std::floor(cameraPosLightSpace.x / texelSize) * texelSize;
  cameraPosLightSpace.y =
      std::floor(cameraPosLightSpace.y / texelSize) * texelSize;

  // Step 4: Transform snapped position back to world space
  glm::mat4 invLightRot = glm::transpose(lightView);
  glm::vec3 snappedCenter =
      glm::vec3(invLightRot * glm::vec4(cameraPosLightSpace, 1.0f));

  // Step 5: Calculate final light position and matrices
  // Light position is in the direction of the sun FROM the scene center
  // (sunDir points TO the sun, so we ADD it to get light position)
  glm::vec3 lightPos = snappedCenter + sunDir * shadowDistance;

  // Orthographic projection for directional light (sun)
  float orthoSize = shadowDistance;
  glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, // left, right
                                         -orthoSize, orthoSize, // bottom, top
                                         Voxel::Config::SHADOW_NEAR_PLANE,
                                         Voxel::Config::SHADOW_FAR_PLANE);

  // Final view matrix with snapped center
  glm::mat4 finalLightView = glm::lookAt(lightPos, snappedCenter, lightUp);

  m_LightSpaceMatrix = lightProjection * finalLightView;

  // === Render to shadow map ===
  m_ShadowMap->Bind();
  glViewport(0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight());
  glClear(GL_DEPTH_BUFFER_BIT);

  // Cull front faces to reduce shadow acne on lit surfaces
  // (Peter panning is less noticeable than acne)
  glCullFace(GL_FRONT);

  // Render all chunks to shadow map
  m_ChunkManager->RenderAllShadow(*m_ShadowShader, m_LightSpaceMatrix);

  // Restore default culling
  glCullFace(GL_BACK);

  m_ShadowMap->Unbind();
}

void Engine::RenderSSAOPass() {
  if (!m_SSAO || !m_SSAO->IsEnabled() || !m_ChunkManager || !m_Camera) {
    return;
  }

  float aspectRatio = static_cast<float>(m_Window->GetWidth()) /
                      static_cast<float>(m_Window->GetHeight());

  // Get projection and view matrices for SSAO
  glm::mat4 projection = m_Camera->GetProjectionMatrix(aspectRatio);
  glm::mat4 view = m_Camera->GetViewMatrix();

  // === DEPTH PRE-PASS ===
  // Render all chunks to depth + normal buffer
  m_SSAO->BeginDepthPass();

  Shader *depthShader = m_SSAO->GetDepthShader();
  if (depthShader) {
    m_ChunkManager->RenderAllDepth(*depthShader, view, projection);
  }

  m_SSAO->EndDepthPass();

  // === SSAO CALCULATION + BLUR ===
  m_SSAO->Calculate(projection, view);

  // Restore main viewport
  glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
}

void Engine::Render() {
  // Get dynamic sky color from SkyRenderer (or default sky blue)
  glm::vec3 skyColor = glm::vec3(0.5f, 0.7f, 1.0f);
  if (m_SkyRenderer) {
    skyColor = m_SkyRenderer->GetSkyColor();
  }

  glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  TextureRegistry &texRegistry = TextureRegistry::Instance();
  if (m_Shader && m_Shader->IsValid() && texRegistry.IsLoaded() && m_Camera &&
      m_ChunkManager) {
    // Calculate aspect ratio
    float aspectRatio = static_cast<float>(m_Window->GetWidth()) /
                        static_cast<float>(m_Window->GetHeight());

    // Update shader uniforms for dynamic lighting
    glm::vec3 cameraPos = m_Camera->GetPosition();
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
    float ambientStrength = 0.35f;

    // Use sun direction and ambient from SkyRenderer if available
    if (m_SkyRenderer) {
      lightDir = m_SkyRenderer->GetSunDirection();
      ambientStrength = m_SkyRenderer->GetAmbientStrength();
    }

    // Determine if shadows should be enabled (only during day when sun is up)
    bool shadowsEnabled = m_ShadowMap && m_ShadowShader && lightDir.y > 0.02f;

    // Calculate shadow strength with smooth fade near horizon
    // Extended fade range: 0 at y=0.02 to 1.0 at y=0.45 for very gradual
    // transition This covers most of the dawn/dusk period for a cinematic
    // effect
    float shadowStrength = 0.0f;
    if (shadowsEnabled) {
      shadowStrength = glm::clamp((lightDir.y - 0.02f) / 0.43f, 0.0f, 1.0f);
      // Apply smoothstep for even smoother easing (S-curve instead of linear)
      shadowStrength =
          shadowStrength * shadowStrength * (3.0f - 2.0f * shadowStrength);
    }

    // === SHADOW PASS ===
    if (shadowsEnabled) {
      RenderShadowPass();
    }

    // === SSAO PASS ===
    bool ssaoEnabled = m_SSAO && m_SSAO->IsEnabled();
    if (ssaoEnabled) {
      RenderSSAOPass();
    }

    // Restore main viewport after off-screen passes
    glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

    // === PASS 0: Render sky (depth write OFF) ===
    if (m_SkyRenderer) {
      glDepthMask(GL_FALSE); // Don't write to depth buffer
      m_SkyRenderer->Render(*m_Camera, aspectRatio);
      glDepthMask(GL_TRUE); // Re-enable depth writing
    }

    // Bind texture array from registry
    texRegistry.GetTextureArray()->Bind(0);

    // Bind shadow map for sampling (slot 1)
    if (shadowsEnabled) {
      m_ShadowMap->BindTexture(1);
    }

    // Bind SSAO texture for sampling (slot 2)
    if (ssaoEnabled) {
      m_SSAO->BindAOTexture(2);
    }

    m_Shader->Bind();
    m_Shader->SetVec3("u_CameraPos", cameraPos);
    m_Shader->SetVec3("u_LightDir", lightDir);
    m_Shader->SetFloat("u_AmbientStrength", ambientStrength);
    m_Shader->SetVec3("u_FogColor", skyColor); // Fog matches sky
    m_Shader->SetBool("u_ShadowsEnabled", shadowsEnabled);
    m_Shader->SetFloat("u_ShadowStrength", shadowStrength); // Smooth fade
    if (shadowsEnabled) {
      m_Shader->SetMat4("u_LightSpaceMatrix", m_LightSpaceMatrix);
    }
    // SSAO uniforms
    m_Shader->SetBool("u_SSAOEnabled", ssaoEnabled);
    m_Shader->SetInt("u_SSAOTex", 2); // SSAO texture in slot 2
    m_Shader->SetVec2("u_ScreenSize",
                      glm::vec2(m_Window->GetWidth(), m_Window->GetHeight()));
    m_Shader->Unbind();

    // === PASS 1: Render opaque geometry ===
    m_ChunkManager->RenderAll(*m_Shader, *m_Camera, aspectRatio);

    // === PASS 1.5: Render block outline (if targeting a block) ===
    if (m_TargetedBlock.hit && m_OutlineShader && m_OutlineShader->IsValid() &&
        m_BlockOutline) {
      glm::mat4 view = m_Camera->GetViewMatrix();
      glm::mat4 proj = m_Camera->GetProjectionMatrix(aspectRatio);
      glm::mat4 viewProj = proj * view;
      m_BlockOutline->Render(m_TargetedBlock.blockPos, *m_OutlineShader,
                             viewProj);
    }

    // === PASS 2: Render transparent water ===
    if (m_WaterShader && m_WaterShader->IsValid()) {
      // Enable alpha blending for water
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Disable depth writes for transparent objects (read only)
      // This prevents water from blocking things behind it in the depth buffer
      glDepthMask(GL_FALSE);

      // Disable backface culling for water so we can see it from underwater
      glDisable(GL_CULL_FACE);

      // Update water shader uniforms
      m_WaterShader->Bind();
      m_WaterShader->SetVec3("u_CameraPos", cameraPos);
      m_WaterShader->SetVec3("u_LightDir", lightDir);
      m_WaterShader->SetFloat("u_AmbientStrength", ambientStrength);
      m_WaterShader->SetVec3("u_FogColor", skyColor);
      m_WaterShader->Unbind();

      // Render water
      m_ChunkManager->RenderWater(*m_WaterShader, *m_Camera, aspectRatio);

      // Restore state
      glEnable(GL_CULL_FACE);
      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
    }

    // === PASS 2.5: Render weather (rain/snow) ===
    if (m_SkyRenderer && m_SkyRenderer->IsWeatherEnabled()) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);

      m_SkyRenderer->RenderWeather(*m_Camera, aspectRatio);

      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
    }

    texRegistry.GetTextureArray()->Unbind();
  }

  // === PASS 3: Render UI (crosshair) ===
  RenderCrosshair();
}

void Engine::UpdateTargetedBlock() {
  if (m_Camera && m_ChunkManager) {
    Ray ray = m_Camera->GetViewRay();
    m_TargetedBlock = Voxel::Raycast(ray, *m_ChunkManager, 8.0f);
  }
}

void Engine::OnMouseButton(int button, int action) {
  // Only process press events
  if (action != GLFW_PRESS) {
    return;
  }

  // Handle title screen button clicks
  if (m_CurrentState == GameState::TITLE_SCREEN &&
      button == GLFW_MOUSE_BUTTON_LEFT) {
    double mouseX, mouseY;
    glfwGetCursorPos(m_Window->GetHandle(), &mouseX, &mouseY);
    if (m_TitleScreen) {
      m_TitleScreen->OnClick(static_cast<float>(mouseX),
                             static_cast<float>(mouseY));
    }
    return;
  }

  // Handle settings screen button clicks
  if (m_CurrentState == GameState::SETTINGS &&
      button == GLFW_MOUSE_BUTTON_LEFT) {
    double mouseX, mouseY;
    glfwGetCursorPos(m_Window->GetHandle(), &mouseX, &mouseY);
    if (m_SettingsScreen) {
      m_SettingsScreen->OnClick(static_cast<float>(mouseX),
                                static_cast<float>(mouseY));
    }
    return;
  }

  // In gameplay mode: only process when cursor is captured
  if (m_CurrentState != GameState::PLAYING || !m_CursorCaptured) {
    return;
  }

  // Check if we're targeting a block
  if (!m_TargetedBlock.hit) {
    return;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    // Break block - set to Air
    m_ChunkManager->SetBlock(m_TargetedBlock.blockPos.x,
                             m_TargetedBlock.blockPos.y,
                             m_TargetedBlock.blockPos.z, Voxel::BLOCK_AIR);
    LOG_DEBUG("Broke block at (" + std::to_string(m_TargetedBlock.blockPos.x) +
              ", " + std::to_string(m_TargetedBlock.blockPos.y) + ", " +
              std::to_string(m_TargetedBlock.blockPos.z) + ")");
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    // Place block at previous position (empty space before the hit block)
    m_ChunkManager->SetBlock(
        m_TargetedBlock.previousPos.x, m_TargetedBlock.previousPos.y,
        m_TargetedBlock.previousPos.z, m_SelectedBlockType);
    LOG_DEBUG("Placed block at (" +
              std::to_string(m_TargetedBlock.previousPos.x) + ", " +
              std::to_string(m_TargetedBlock.previousPos.y) + ", " +
              std::to_string(m_TargetedBlock.previousPos.z) + ")");
  }
}

void Engine::SetupCrosshair() {
  // Crosshair size in NDC (normalized device coordinates)
  // We'll update these dynamically in RenderCrosshair to account for aspect ratio
  // Using a smaller base size (in terms of Y-axis NDC)
  const float size = 0.015f; // Smaller arm length

  // Crosshair vertices: horizontal line + vertical line
  // Drawing as GL_LINES (pairs of vertices)
  // Note: These will be dynamically updated in RenderCrosshair for aspect ratio
  float vertices[] = {
      // Horizontal line (left to right)
      -size, 0.0f, // Left point
      size, 0.0f,  // Right point
                   // Vertical line (bottom to top)
      0.0f, -size, // Bottom point
      0.0f, size   // Top point
  };

  glGenVertexArrays(1, &m_CrosshairVAO);
  glGenBuffers(1, &m_CrosshairVBO);

  glBindVertexArray(m_CrosshairVAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_CrosshairVBO);
  // Use GL_DYNAMIC_DRAW since we update vertices each frame for aspect ratio
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  // Position attribute (location 0)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  LOG_INFO("Crosshair initialized");
}

void Engine::RenderCrosshair() {
  if (!m_UIShader || !m_UIShader->IsValid() || m_CrosshairVAO == 0) {
    return;
  }

  // Calculate aspect ratio and correct crosshair size
  float aspectRatio = static_cast<float>(m_Window->GetWidth()) /
                      static_cast<float>(m_Window->GetHeight());
  const float size = 0.015f; // Base size in NDC (Y-axis)
  
  // Correct horizontal size for aspect ratio so crosshair appears square
  float hSize = size / aspectRatio;
  float vSize = size;

  // Update vertices dynamically for aspect ratio correction
  float vertices[] = {
      // Horizontal line (left to right)
      -hSize, 0.0f,
      hSize, 0.0f,
      // Vertical line (bottom to top)
      0.0f, -vSize,
      0.0f, vSize
  };

  // Update VBO with corrected vertices
  glBindBuffer(GL_ARRAY_BUFFER, m_CrosshairVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  // Disable depth test for UI
  glDisable(GL_DEPTH_TEST);

  // Enable blending for smooth edges (optional)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set line width (may not work on all systems, but worth trying)
  glLineWidth(2.0f);

  m_UIShader->Bind();
  // White crosshair with slight transparency
  m_UIShader->SetVec4("u_Color", glm::vec4(1.0f, 1.0f, 1.0f, 0.9f));

  glBindVertexArray(m_CrosshairVAO);
  glDrawArrays(GL_LINES, 0, 4); // 4 vertices = 2 lines
  glBindVertexArray(0);

  m_UIShader->Unbind();

  // Restore state
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}

void Engine::CleanupCrosshair() {
  if (m_CrosshairVAO != 0) {
    glDeleteVertexArrays(1, &m_CrosshairVAO);
    m_CrosshairVAO = 0;
  }
  if (m_CrosshairVBO != 0) {
    glDeleteBuffers(1, &m_CrosshairVBO);
    m_CrosshairVBO = 0;
  }
}

// ============================================================================
// STATE MANAGEMENT METHODS
// ============================================================================

void Engine::UpdateTitleScreen(float /*deltaTime*/) {
  // Get mouse position for button hover
  double mouseX, mouseY;
  glfwGetCursorPos(m_Window->GetHandle(), &mouseX, &mouseY);

  if (m_TitleScreen) {
    m_TitleScreen->Update(static_cast<float>(mouseX),
                          static_cast<float>(mouseY));
  }
}

void Engine::RenderTitleScreen() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_TitleScreen) {
    m_TitleScreen->Render(m_Window->GetWidth(), m_Window->GetHeight());
  }
}

void Engine::UpdateSettingsScreen(float /*deltaTime*/) {
  // Get mouse position for button hover
  double mouseX, mouseY;
  glfwGetCursorPos(m_Window->GetHandle(), &mouseX, &mouseY);

  if (m_SettingsScreen) {
    m_SettingsScreen->Update(static_cast<float>(mouseX),
                             static_cast<float>(mouseY));
  }
}

void Engine::RenderSettingsScreen() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_SettingsScreen) {
    m_SettingsScreen->Render(m_Window->GetWidth(), m_Window->GetHeight());
  }
}

void Engine::UpdateLoadingScreen(float deltaTime) {
  // Start world setup if not already started
  if (!m_WorldSetupStarted) {
    m_WorldSetupStarted = true;
    m_LoadingProgress = 0.0f;

    if (m_LoadingScreen) {
      m_LoadingScreen->SetStatus("Initializing shaders...");
      m_LoadingScreen->SetProgress(0.0f);
    }
  }

  // Simulate loading progress (in a real implementation, this would be async)
  // For now, we do incremental setup each frame
  if (m_LoadingProgress < 0.1f) {
    // Create shaders
    if (!m_Shader) {
      SetupWorld(); // This does all the heavy lifting
    }
    m_LoadingProgress = 1.0f; // World is setup synchronously for now

    if (m_LoadingScreen) {
      m_LoadingScreen->SetStatus("World ready!");
      m_LoadingScreen->SetProgress(1.0f);
    }
  }

  // Check if loading complete
  if (m_LoadingProgress >= 1.0f) {
    // Small delay to show completion
    static float completionDelay = 0.0f;
    completionDelay += deltaTime;
    if (completionDelay > 0.5f) {
      completionDelay = 0.0f;
      TransitionToState(GameState::PLAYING);
    }
  }
}

void Engine::RenderLoadingScreen() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_LoadingScreen) {
    m_LoadingScreen->Render(m_Window->GetWidth(), m_Window->GetHeight());
  }
}

void Engine::TransitionToState(GameState newState) {
  GameState oldState = m_CurrentState;
  (void)oldState; // Unused but available for logging
  m_CurrentState = newState;

  switch (newState) {
  case GameState::TITLE_SCREEN:
    LOG_INFO("Transitioning to Title Screen");
    glfwSetInputMode(m_Window->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_CursorCaptured = false;
    break;

  case GameState::SETTINGS:
    LOG_INFO("Transitioning to Settings Screen");
    glfwSetInputMode(m_Window->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_CursorCaptured = false;
    break;

  case GameState::LOADING:
    LOG_INFO("Transitioning to Loading Screen");
    glfwSetInputMode(m_Window->GetHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_WorldSetupStarted = false;
    m_LoadingProgress = 0.0f;
    break;

  case GameState::PLAYING:
    LOG_INFO("Transitioning to Gameplay");
    LOG_INFO(
        "Controls: WASD to move, Space/Shift for up/down, M to capture mouse");
    LOG_INFO("Left-click to break blocks, Right-click to place blocks");
    // Cursor will be captured when player presses M
    break;
  }
}

} // namespace Core
