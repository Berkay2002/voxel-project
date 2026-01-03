#include "core/Window.h"
#include "core/Logger.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace Core {

Window::Window(int width, int height, const std::string &title)
    : m_Width(width), m_Height(height) {

  LOG_INFO("Initializing GLFW...");

  if (!glfwInit()) {
    LOG_ERROR("Failed to initialize GLFW");
    throw std::runtime_error("Failed to initialize GLFW");
  }

  // OpenGL 4.6 Core Profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    LOG_ERROR("Failed to create GLFW window");
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(m_Window);
  glfwSetWindowUserPointer(m_Window, this);

  // Set callbacks
  glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
  glfwSetKeyCallback(m_Window, KeyCallback);

  // Enable VSync
  glfwSwapInterval(1);

  LOG_INFO("Window created: " + std::to_string(width) + "x" +
           std::to_string(height));
}

Window::~Window() {
  if (m_Window) {
    glfwDestroyWindow(m_Window);
  }
  glfwTerminate();
  LOG_INFO("Window destroyed, GLFW terminated");
}

bool Window::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

void Window::SwapBuffers() {
  glfwSwapBuffers(m_Window);
  glfwPollEvents();
}

void Window::SetResizeCallback(ResizeCallback callback) {
  m_ResizeCallback = std::move(callback);
}

void Window::FramebufferSizeCallback(GLFWwindow *window, int width,
                                     int height) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->m_Width = width;
    self->m_Height = height;
    if (self->m_ResizeCallback) {
      self->m_ResizeCallback(width, height);
    }
  }
}

void Window::KeyCallback(GLFWwindow *window, int key, int /*scancode*/,
                         int action, int /*mods*/) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

} // namespace Core
