#include "core/Engine.h"
#include "core/Camera.h"
#include "core/IndexBuffer.h"
#include "core/Logger.h"
#include "core/Shader.h"
#include "core/Texture.h"
#include "core/VertexArray.h"
#include "core/VertexBuffer.h"
#include "core/Window.h"

// Voxel system
#include "world/Block.h"
#include "world/Chunk.h"
#include "world/ChunkMeshBuilder.h"

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

  // Enable backface culling for performance
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Create camera - position it to view the chunk
  m_Camera = std::make_unique<Camera>(glm::vec3(8.0f, 20.0f, 30.0f));

  // Setup chunk rendering
  SetupChunk();

  LOG_INFO("Engine initialized successfully!");
  LOG_INFO("Controls: WASD to move, Space/Shift for up/down");
  LOG_INFO("Press M to capture mouse for looking around, ESC to quit");
}

Engine::~Engine() { LOG_INFO("Engine shutting down..."); }

void Engine::SetupChunk() {
  // Create and populate a test chunk with simple terrain
  m_Chunk = std::make_unique<Voxel::Chunk>();

  // Generate simple height-based terrain
  for (int x = 0; x < Voxel::CHUNK_WIDTH; ++x) {
    for (int z = 0; z < Voxel::CHUNK_DEPTH; ++z) {
      // Simple height variation (5-10 blocks high)
      int height = 5 + (x + z) % 6;

      for (int y = 0; y < height; ++y) {
        if (y == height - 1) {
          // Top layer is grass
          m_Chunk->SetBlock(x, y, z, Voxel::BlockType::Grass);
        } else if (y >= height - 4) {
          // Next 3 layers are dirt
          m_Chunk->SetBlock(x, y, z, Voxel::BlockType::Dirt);
        } else {
          // Everything below is stone
          m_Chunk->SetBlock(x, y, z, Voxel::BlockType::Stone);
        }
      }
    }
  }

  // Build mesh from chunk data with face culling
  Voxel::ChunkMeshBuilder meshBuilder;
  Voxel::ChunkMesh mesh = meshBuilder.BuildMesh(*m_Chunk);

  LOG_INFO("Chunk mesh built: " + std::to_string(mesh.vertices.size()) + 
           " vertices, " + std::to_string(mesh.indices.size()) + " indices");
  LOG_INFO("Face culling reduced faces to only visible surfaces!");

  if (mesh.IsEmpty()) {
    LOG_ERROR("Chunk mesh is empty!");
    return;
  }

  // Create textured shader
  m_Shader = std::make_unique<Shader>("assets/shaders/textured.vert",
                                       "assets/shaders/textured.frag");

  if (!m_Shader->IsValid()) {
    LOG_ERROR("Failed to create shader for chunk");
    return;
  }

  // Load texture (using grass block for now - single texture for all blocks)
  m_Texture = std::make_unique<Texture>("assets/textures/blocks/grass_block.png");

  if (!m_Texture->IsValid()) {
    LOG_ERROR("Failed to load texture for chunk");
    return;
  }

  // Create VAO
  m_VAO = std::make_unique<VertexArray>();

  // Create VBO from mesh vertices
  // ChunkVertex has: vec3 position (12 bytes) + vec2 uv (8 bytes) + vec3 normal (12 bytes) = 32 bytes
  m_VBO = std::make_unique<VertexBuffer>(
      mesh.vertices.data(),
      mesh.vertices.size() * sizeof(Voxel::ChunkVertex));

  // Create IBO from mesh indices
  m_IBO = std::make_unique<IndexBuffer>(
      mesh.indices.data(),
      static_cast<unsigned int>(mesh.indices.size()));

  m_ChunkIndexCount = static_cast<unsigned int>(mesh.indices.size());

  // Setup vertex attributes: position (3 floats) + uv (2 floats) + normal (3 floats)
  // Stride = sizeof(ChunkVertex) = 32 bytes
  std::vector<VertexAttribute> attributes = {
      {0, 3, GL_FLOAT, false, sizeof(Voxel::ChunkVertex), 0},                        // Position
      {1, 2, GL_FLOAT, false, sizeof(Voxel::ChunkVertex), offsetof(Voxel::ChunkVertex, uv)},    // UV
      {2, 3, GL_FLOAT, false, sizeof(Voxel::ChunkVertex), offsetof(Voxel::ChunkVertex, normal)} // Normal
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

  LOG_INFO("Chunk setup complete - rendering " + 
           std::to_string(m_ChunkIndexCount / 3) + " triangles");
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
  // Clear with sky blue color
  glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (m_Shader && m_Shader->IsValid() && m_VAO && m_IBO && m_Texture && m_Camera) {
    m_Shader->Bind();

    // Calculate MVP matrix
    float aspectRatio = static_cast<float>(m_Window->GetWidth()) / 
                        static_cast<float>(m_Window->GetHeight());
    
    // No model transformation needed - chunk is at origin
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = m_Camera->GetViewProjectionMatrix(aspectRatio) * model;
    m_Shader->SetMat4("u_MVP", mvp);

    // Bind texture
    m_Texture->Bind(0);

    // Draw chunk
    m_VAO->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_ChunkIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    m_VAO->Unbind();

    m_Texture->Unbind();
    m_Shader->Unbind();
  }
}

} // namespace Core
