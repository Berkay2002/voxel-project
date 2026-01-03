#include "core/Engine.h"
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

  // Setup rendering resources
  SetupQuad();

  LOG_INFO("Engine initialized successfully!");
}

Engine::~Engine() { LOG_INFO("Engine shutting down..."); }

void Engine::SetupQuad() {
  // Quad vertices: position (x, y, z) + texture coords (u, v)
  // clang-format off
  float vertices[] = {
    // Position          // TexCoord
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  // Bottom left
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  // Bottom right
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,  // Top right
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f   // Top left
  };
  // clang-format on

  unsigned int indices[] = {
      0, 1, 2,  // First triangle
      2, 3, 0   // Second triangle
  };

  // Create textured shader
  m_Shader = std::make_unique<Shader>("assets/shaders/textured.vert",
                                       "assets/shaders/textured.frag");

  if (!m_Shader->IsValid()) {
    LOG_ERROR("Failed to create shader for quad");
    return;
  }

  // Load texture
  m_Texture = std::make_unique<Texture>("assets/textures/debug_texture.png");

  if (!m_Texture->IsValid()) {
    LOG_ERROR("Failed to load texture for quad");
    return;
  }

  // Create VAO
  m_VAO = std::make_unique<VertexArray>();

  // Create VBO
  m_VBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));

  // Create IBO
  m_IBO = std::make_unique<IndexBuffer>(indices, 6);

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
  m_Shader->SetInt("u_Texture", 0);  // Texture unit 0
  m_Shader->Unbind();

  LOG_INFO("Textured quad setup complete");
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

  if (m_Shader && m_Shader->IsValid() && m_VAO && m_IBO && m_Texture) {
    m_Shader->Bind();

    // Set MVP matrix (identity for 2D, no transforms)
    glm::mat4 mvp = glm::mat4(1.0f);
    m_Shader->SetMat4("u_MVP", mvp);

    // Bind texture
    m_Texture->Bind(0);

    // Draw quad
    m_VAO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_IBO->GetCount()),
                   GL_UNSIGNED_INT, nullptr);
    m_VAO->Unbind();

    m_Texture->Unbind();
    m_Shader->Unbind();
  }
}

} // namespace Core
