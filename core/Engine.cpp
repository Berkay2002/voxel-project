#include "core/Engine.h"
#include "core/IndexBuffer.h"
#include "core/Logger.h"
#include "core/Shader.h"
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

  // Setup rendering resources
  SetupTriangle();

  LOG_INFO("Engine initialized successfully!");
}

Engine::~Engine() { LOG_INFO("Engine shutting down..."); }

void Engine::SetupTriangle() {
  // Triangle vertices: position (x, y, z)
  // clang-format off
  float vertices[] = {
    // Position
    -0.5f, -0.5f, 0.0f,  // Bottom left
     0.5f, -0.5f, 0.0f,  // Bottom right
     0.0f,  0.5f, 0.0f   // Top center
  };
  // clang-format on

  unsigned int indices[] = {0, 1, 2};

  // Create shader
  m_Shader = std::make_unique<Shader>("assets/shaders/basic.vert",
                                       "assets/shaders/basic.frag");

  if (!m_Shader->IsValid()) {
    LOG_ERROR("Failed to create shader for triangle");
    return;
  }

  // Create VAO
  m_VAO = std::make_unique<VertexArray>();

  // Create VBO
  m_VBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));

  // Create IBO
  m_IBO = std::make_unique<IndexBuffer>(indices, 3);

  // Setup vertex attributes: position (3 floats)
  std::vector<VertexAttribute> attributes = {
      {0, 3, GL_FLOAT, false, 3 * sizeof(float), 0}  // Position
  };
  m_VAO->AddVertexBuffer(*m_VBO, attributes);

  // Bind IBO to VAO
  m_VAO->Bind();
  m_IBO->Bind();
  m_VAO->Unbind();

  LOG_INFO("Triangle setup complete");
}

void Engine::Run() {
  LOG_INFO("Starting main loop...");

  while (!m_Window->ShouldClose()) {
    Update();
    Render();
    m_Window->SwapBuffers();
  }

  LOG_INFO("Main loop ended");
}

void Engine::Update() {
  // Future: Game logic, input handling, etc.
}

void Engine::Render() {
  // Clear with teal color
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_Shader && m_Shader->IsValid() && m_VAO && m_IBO) {
    m_Shader->Bind();

    // Set MVP matrix (identity for 2D, no transforms)
    glm::mat4 mvp = glm::mat4(1.0f);
    m_Shader->SetMat4("u_MVP", mvp);

    // Set color (orange-red)
    m_Shader->SetVec4("u_Color", glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));

    // Draw triangle
    m_VAO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_IBO->GetCount()),
                   GL_UNSIGNED_INT, nullptr);
    m_VAO->Unbind();

    m_Shader->Unbind();
  }
}

} // namespace Core
