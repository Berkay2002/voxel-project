#pragma once

#include <memory>

namespace Core {

class Window;
class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;

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

private:
  void Update();
  void Render();
  void SetupTriangle();

  std::unique_ptr<Window> m_Window;
  
  // Rendering resources
  std::unique_ptr<Shader> m_Shader;
  std::unique_ptr<VertexArray> m_VAO;
  std::unique_ptr<VertexBuffer> m_VBO;
  std::unique_ptr<IndexBuffer> m_IBO;
};

} // namespace Core
