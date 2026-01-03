#pragma once

#include <memory>

namespace Core {

class Window;

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

  std::unique_ptr<Window> m_Window;
};

} // namespace Core
