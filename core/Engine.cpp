#include "core/Engine.h"
#include "core/Camera.h"
#include "core/Logger.h"
#include "core/Shader.h"
#include "core/TextureArray.h"
#include "core/TextureRegistry.h"
#include "core/Window.h"

// Voxel system
#include "world/BlockRegistry.h"
#include "world/ChunkManager.h"
#include "world/SpaghettiCaveCarver.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace Core {

// Static pointer for GLFW callback access (can't use glfwSetWindowUserPointer, 
// it's already used by Window class for resize callback)
static Engine* s_Instance = nullptr;

Engine::Engine() {
  LOG_INFO("Initializing Engine...");

  // Create window (800x600)
  m_Window = std::make_unique<Window>(800, 600, "Voxel Engine");

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

  // Set viewport resize callback
  m_Window->SetResizeCallback([](int width, int height) {
    glViewport(0, 0, width, height);
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

  // Setup mouse button callback for block interaction
  // Note: We can't use glfwSetWindowUserPointer here because Window.cpp already uses it
  // Instead, we store a static pointer to this Engine instance
  s_Instance = this;
  glfwSetMouseButtonCallback(m_Window->GetHandle(), [](GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    if (s_Instance) {
      s_Instance->OnMouseButton(button, action);
    }
  });

  // Setup world rendering
  SetupWorld();

  LOG_INFO("Engine initialized successfully!");
  LOG_INFO("Controls: WASD to move, Space/Shift for up/down, M to capture mouse");
  LOG_INFO("Left-click to break blocks, Right-click to place blocks");
}

Engine::~Engine() { 
  CleanupCrosshair();
  s_Instance = nullptr;  // Clear static pointer
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

  // Setup crosshair
  SetupCrosshair();

  // =========================================================================
  // PHASE 11: Data-driven texture and block loading via registries
  // =========================================================================
  
  // Define textures to load (order determines layer indices)
  // This list should include all textures referenced in blocks.json
  std::vector<std::string> textureList = {
      "grass_block_top",    // Layer 0
      "dirt",               // Layer 1
      "grass_block_side",   // Layer 2
      "stone",              // Layer 3
      "blue_ice",           // Layer 4 (water placeholder - water_still is animated spritesheet)
      "bedrock",            // Layer 5
      "sand",               // Layer 6
      "gravel",             // Layer 7
      "cobblestone",        // Layer 8
      "oak_log_top",        // Layer 9
      "oak_log",            // Layer 10
      "oak_planks",         // Layer 11
      "oak_leaves",         // Layer 12
      "coal_ore",           // Layer 13
      "iron_ore",           // Layer 14
      "gold_ore",           // Layer 15
      "diamond_ore",        // Layer 16
      "copper_ore",         // Layer 17
      "emerald_ore"         // Layer 18
  };
  
  // Load textures via TextureRegistry
  TextureRegistry& texRegistry = TextureRegistry::Instance();
  if (!texRegistry.LoadTextures(textureList, "assets/textures/blocks/")) {
    LOG_ERROR("Failed to load textures via TextureRegistry");
    return;
  }
  
  // Load block definitions via BlockRegistry
  Voxel::BlockRegistry& blockRegistry = Voxel::BlockRegistry::Instance();
  if (!blockRegistry.LoadFromFile("assets/config/blocks.json", texRegistry)) {
    LOG_ERROR("Failed to load block definitions via BlockRegistry");
    return;
  }

  // Set lit shader uniforms
  m_Shader->Bind();
  m_Shader->SetInt("u_TextureArray", 0);  // Texture array in slot 0
  // Sun direction: slightly angled from above-right
  m_Shader->SetVec3("u_LightDir", glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));
  // Ambient strength: prevents pure black shadows
  m_Shader->SetFloat("u_AmbientStrength", 0.35f);
  m_Shader->Unbind();

  // Set water shader uniforms
  m_WaterShader->Bind();
  m_WaterShader->SetInt("u_TextureArray", 0);  // Same texture array
  m_WaterShader->SetVec3("u_LightDir", glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));
  m_WaterShader->SetFloat("u_AmbientStrength", 0.35f);
  m_WaterShader->SetFloat("u_WaterAlpha", 0.7f);  // Water transparency
  m_WaterShader->SetFloat("u_Time", 0.0f);        // For optional animation
  m_WaterShader->Unbind();

  // Create chunk manager
  m_ChunkManager = std::make_unique<Voxel::ChunkManager>();

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

  // Keyboard input
  bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
  bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
  bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
  bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
  bool up = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
  bool down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  bool sprint = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

  m_Camera->ProcessKeyboard(deltaTime, forward, backward, left, right, up, down, sprint);

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
    float yOffset = m_LastY - static_cast<float>(ypos);  // Reversed: y-coords go bottom to top

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

    Update(deltaTime);
    Render();
    m_Window->SwapBuffers();
  }

  LOG_INFO("Main loop ended");
}

void Engine::Update(float deltaTime) {
  glfwPollEvents();
  ProcessInput(deltaTime);

  // Update targeted block for interaction (raycast from camera)
  UpdateTargetedBlock();

  // Update chunk loading based on camera position
  if (m_ChunkManager && m_Camera) {
    m_ChunkManager->Update(m_Camera->GetPosition());
    // Process completed async mesh generations (upload to GPU on main thread)
    m_ChunkManager->ProcessPendingMeshes();
  }
}

void Engine::Render() {
  // Clear with sky blue color
  glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  TextureRegistry& texRegistry = TextureRegistry::Instance();
  if (m_Shader && m_Shader->IsValid() && texRegistry.IsLoaded() && m_Camera && m_ChunkManager) {
    // Calculate aspect ratio
    float aspectRatio = static_cast<float>(m_Window->GetWidth()) / 
                        static_cast<float>(m_Window->GetHeight());

    // Bind texture array from registry
    texRegistry.GetTextureArray()->Bind(0);

    // === PASS 1: Render opaque geometry ===
    m_ChunkManager->RenderAll(*m_Shader, *m_Camera, aspectRatio);

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
      
      // Render water
      m_ChunkManager->RenderWater(*m_WaterShader, *m_Camera, aspectRatio);
      
      // Restore state
      glEnable(GL_CULL_FACE);
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
  // Only process press events when cursor is captured
  if (action != GLFW_PRESS || !m_CursorCaptured) {
    return;
  }
  
  // Check if we're targeting a block
  if (!m_TargetedBlock.hit) {
    return;
  }
  
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    // Break block - set to Air
    m_ChunkManager->SetBlock(
        m_TargetedBlock.blockPos.x,
        m_TargetedBlock.blockPos.y,
        m_TargetedBlock.blockPos.z,
        Voxel::BLOCK_AIR
    );
    LOG_DEBUG("Broke block at (" + 
        std::to_string(m_TargetedBlock.blockPos.x) + ", " +
        std::to_string(m_TargetedBlock.blockPos.y) + ", " +
        std::to_string(m_TargetedBlock.blockPos.z) + ")");
  } 
  else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    // Place block at previous position (empty space before the hit block)
    m_ChunkManager->SetBlock(
        m_TargetedBlock.previousPos.x,
        m_TargetedBlock.previousPos.y,
        m_TargetedBlock.previousPos.z,
        m_SelectedBlockType
    );
    LOG_DEBUG("Placed block at (" +
        std::to_string(m_TargetedBlock.previousPos.x) + ", " +
        std::to_string(m_TargetedBlock.previousPos.y) + ", " +
        std::to_string(m_TargetedBlock.previousPos.z) + ")");
  }
}

void Engine::SetupCrosshair() {
  // Crosshair size in NDC (normalized device coordinates)
  // Adjust these for larger/smaller crosshair
  const float size = 0.02f;   // Length of each arm
  const float gap = 0.005f;   // Gap in the center (optional, set to 0 for solid +)
  
  // Crosshair vertices: horizontal line + vertical line
  // Drawing as GL_LINES (pairs of vertices)
  float vertices[] = {
      // Horizontal line (left to right)
      -size, 0.0f,    // Left point
       size, 0.0f,    // Right point
      // Vertical line (bottom to top)
       0.0f, -size,   // Bottom point
       0.0f,  size    // Top point
  };
  
  glGenVertexArrays(1, &m_CrosshairVAO);
  glGenBuffers(1, &m_CrosshairVBO);
  
  glBindVertexArray(m_CrosshairVAO);
  
  glBindBuffer(GL_ARRAY_BUFFER, m_CrosshairVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  // Position attribute (location 0)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  glBindVertexArray(0);
  
  LOG_INFO("Crosshair initialized");
}

void Engine::RenderCrosshair() {
  if (!m_UIShader || !m_UIShader->IsValid() || m_CrosshairVAO == 0) {
    return;
  }
  
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
  glDrawArrays(GL_LINES, 0, 4);  // 4 vertices = 2 lines
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

} // namespace Core
