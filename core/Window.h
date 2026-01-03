#pragma once

#include <functional>
#include <string>


struct GLFWwindow;

namespace Core {

class Window {
public:
  Window(int width, int height, const std::string &title);
  ~Window();

  // Non-copyable, non-movable
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  [[nodiscard]] bool ShouldClose() const;
  void SwapBuffers();

  [[nodiscard]] GLFWwindow *GetHandle() const { return m_Window; }
  [[nodiscard]] int GetWidth() const { return m_Width; }
  [[nodiscard]] int GetHeight() const { return m_Height; }

  // Callbacks
  using ResizeCallback = std::function<void(int, int)>;
  void SetResizeCallback(ResizeCallback callback);

private:
  GLFWwindow *m_Window = nullptr;
  int m_Width;
  int m_Height;
  ResizeCallback m_ResizeCallback;

  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
};

} // namespace Core
