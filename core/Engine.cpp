#include "core/Engine.h"
#include "core/Camera.h"
#include "core/Logger.h"
#include "core/Shader.h"
#include "core/TextureArray.h"
#include "core/Window.h"

// Voxel system
#include "world/ChunkManager.h"
#include "world/SpaghettiCaveCarver.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace Core {

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

  // Setup world rendering
  SetupWorld();

  LOG_INFO("Engine initialized successfully!");
  LOG_INFO("Controls: WASD to move, Space/Shift for up/down");
  LOG_INFO("Press M to capture mouse for looking around, ESC to quit");
}

Engine::~Engine() { LOG_INFO("Engine shutting down..."); }

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

  // Load block textures into texture array
  // Layer order matches GetTextureIndex() in Block.h:
  //   0 = grass_block.png (grass top)
  //   1 = dirt_block.png
  //   2 = grass_block_side.png
  //   3 = stone_block.png
  m_TextureArray = std::make_unique<TextureArray>(std::vector<std::string>{
      "assets/textures/blocks/grass_block.png",      // Layer 0: Grass top
      "assets/textures/blocks/dirt_block.png",       // Layer 1: Dirt
      "assets/textures/blocks/grass_block_side.png", // Layer 2: Grass side
      "assets/textures/blocks/stone_block.png"       // Layer 3: Stone
  });

  if (!m_TextureArray->IsValid()) {
    LOG_ERROR("Failed to load texture array for world");
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

  LOG_INFO("World setup complete with ChunkManager, texture array, and water system");
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

  if (m_Shader && m_Shader->IsValid() && m_TextureArray && m_Camera && m_ChunkManager) {
    // Calculate aspect ratio
    float aspectRatio = static_cast<float>(m_Window->GetWidth()) / 
                        static_cast<float>(m_Window->GetHeight());

    // Bind texture array
    m_TextureArray->Bind(0);

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

    m_TextureArray->Unbind();
  }
}

} // namespace Core
