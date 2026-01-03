#include "core/Engine.h"
#include "core/Camera.h"
#include "core/Logger.h"
#include "core/Shader.h"
#include "core/Texture.h"
#include "core/Window.h"

// Voxel system
#include "world/ChunkManager.h"

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

  // Create camera - position it to view the terrain (height ~64 blocks)
  m_Camera = std::make_unique<Camera>(glm::vec3(8.0f, 80.0f, 40.0f));

  // Setup world rendering
  SetupWorld();

  LOG_INFO("Engine initialized successfully!");
  LOG_INFO("Controls: WASD to move, Space/Shift for up/down");
  LOG_INFO("Press M to capture mouse for looking around, ESC to quit");
}

Engine::~Engine() { LOG_INFO("Engine shutting down..."); }

void Engine::SetupWorld() {
  // Create textured shader
  m_Shader = std::make_unique<Shader>("assets/shaders/textured.vert",
                                       "assets/shaders/textured.frag");

  if (!m_Shader->IsValid()) {
    LOG_ERROR("Failed to create shader for world");
    return;
  }

  // Load texture (using grass block for now - single texture for all blocks)
  m_Texture = std::make_unique<Texture>("assets/textures/blocks/grass_block.png");

  if (!m_Texture->IsValid()) {
    LOG_ERROR("Failed to load texture for world");
    return;
  }

  // Set texture uniform
  m_Shader->Bind();
  m_Shader->SetInt("u_Texture", 0);
  m_Shader->Unbind();

  // Create chunk manager
  m_ChunkManager = std::make_unique<Voxel::ChunkManager>();

  LOG_INFO("World setup complete with ChunkManager");
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

  m_Camera->ProcessKeyboard(deltaTime, forward, backward, left, right, up, down);

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

  if (m_Shader && m_Shader->IsValid() && m_Texture && m_Camera && m_ChunkManager) {
    // Bind texture
    m_Texture->Bind(0);

    // Calculate aspect ratio
    float aspectRatio = static_cast<float>(m_Window->GetWidth()) / 
                        static_cast<float>(m_Window->GetHeight());

    // Render all chunks
    m_ChunkManager->RenderAll(*m_Shader, *m_Camera, aspectRatio);

    m_Texture->Unbind();
  }
}

} // namespace Core
