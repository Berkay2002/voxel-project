#include "core/Engine.h"
#include "core/Camera.h"
#include "core/IndexBuffer.h"
#include "core/Logger.h"
#include "core/Shader.h"
#include "core/Texture.h"
#include "core/VertexArray.h"
#include "core/VertexBuffer.h"
#include "core/Window.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

  // Create camera
  m_Camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));

  // Setup rendering resources
  SetupCube();

  LOG_INFO("Engine initialized successfully!");
  LOG_INFO("Controls: WASD to move, Space/Shift for up/down");
  LOG_INFO("Press M to capture mouse for looking around, ESC to quit");
}

Engine::~Engine() { LOG_INFO("Engine shutting down..."); }

void Engine::SetupCube() {
  // Cube vertices: position (x, y, z) + texture coords (u, v)
  // clang-format off
  float vertices[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    // Back face
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    // Top face
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    // Bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    // Right face
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    // Left face
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
  };
  // clang-format on

  unsigned int indices[] = {
      // Front
      0, 1, 2, 2, 3, 0,
      // Back
      4, 5, 6, 6, 7, 4,
      // Top
      8, 9, 10, 10, 11, 8,
      // Bottom
      12, 13, 14, 14, 15, 12,
      // Right
      16, 17, 18, 18, 19, 16,
      // Left
      20, 21, 22, 22, 23, 20
  };

  // Create textured shader
  m_Shader = std::make_unique<Shader>("assets/shaders/textured.vert",
                                       "assets/shaders/textured.frag");

  if (!m_Shader->IsValid()) {
    LOG_ERROR("Failed to create shader for cube");
    return;
  }

  // Load texture (using grass block for the cube)
  m_Texture = std::make_unique<Texture>("assets/textures/blocks/grass_block.png");

  if (!m_Texture->IsValid()) {
    LOG_ERROR("Failed to load texture for cube");
    return;
  }

  // Create VAO
  m_VAO = std::make_unique<VertexArray>();

  // Create VBO
  m_VBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));

  // Create IBO
  m_IBO = std::make_unique<IndexBuffer>(indices, 36);

  // Setup vertex attributes: position (3 floats) + texcoord (2 floats)
  std::vector<VertexAttribute> attributes = {
      {0, 3, GL_FLOAT, false, 5 * sizeof(float), 0},                     // Position
      {1, 2, GL_FLOAT, false, 5 * sizeof(float), 3 * sizeof(float)}      // TexCoord
  };
  m_VAO->AddVertexBuffer(*m_VBO, attributes);

  // Bind IBO to VAO
  m_VAO->Bind();
  m_IBO->Bind();
  m_VAO->Unbind();

  // Set texture uniform
  m_Shader->Bind();
  m_Shader->SetInt("u_Texture", 0);
  m_Shader->Unbind();

  LOG_INFO("3D Cube setup complete");
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
}

void Engine::Render() {
  // Clear with teal color
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_Shader && m_Shader->IsValid() && m_VAO && m_IBO && m_Texture && m_Camera) {
    m_Shader->Bind();

    // Calculate MVP matrix
    float aspectRatio = static_cast<float>(m_Window->GetWidth()) / 
                        static_cast<float>(m_Window->GetHeight());
    
    glm::mat4 model = glm::mat4(1.0f);
    // Rotate the cube slowly for visual effect
    model = glm::rotate(model, static_cast<float>(glfwGetTime()) * 0.5f, 
                        glm::vec3(0.0f, 1.0f, 0.0f));
    
    glm::mat4 mvp = m_Camera->GetViewProjectionMatrix(aspectRatio) * model;
    m_Shader->SetMat4("u_MVP", mvp);

    // Bind texture
    m_Texture->Bind(0);

    // Draw cube
    m_VAO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_IBO->GetCount()),
                   GL_UNSIGNED_INT, nullptr);
    m_VAO->Unbind();

    m_Texture->Unbind();
    m_Shader->Unbind();
  }
}

} // namespace Core
