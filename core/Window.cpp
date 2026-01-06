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

  // Store initial windowed position
  DisplayConfig &config = DisplayConfig::Instance();
  config.windowedWidth = width;
  config.windowedHeight = height;
  glfwGetWindowPos(m_Window, &config.windowPosX, &config.windowPosY);

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

GLFWmonitor *Window::GetPrimaryMonitor() const {
  return glfwGetPrimaryMonitor();
}

void Window::GetMonitorSize(int &width, int &height) const {
  GLFWmonitor *monitor = GetPrimaryMonitor();
  if (monitor) {
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (mode) {
      width = mode->width;
      height = mode->height;
      return;
    }
  }
  // Fallback
  width = 1920;
  height = 1080;
}

void Window::SetVSync(bool enabled) {
  glfwSwapInterval(enabled ? 1 : 0);
  DisplayConfig::Instance().vsyncEnabled = enabled;
  LOG_INFO(std::string("VSync ") + (enabled ? "enabled" : "disabled"));
}

void Window::SetWindowMode(WindowMode mode) {
  if (mode == m_WindowMode) {
    return; // No change needed
  }

  DisplayConfig &config = DisplayConfig::Instance();
  GLFWmonitor *monitor = GetPrimaryMonitor();
  const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);

  // Save current windowed position/size before switching away from windowed
  if (m_WindowMode == WindowMode::WINDOWED) {
    glfwGetWindowPos(m_Window, &config.windowPosX, &config.windowPosY);
    glfwGetWindowSize(m_Window, &config.windowedWidth, &config.windowedHeight);
  }

  switch (mode) {
  case WindowMode::WINDOWED: {
    // Switch to windowed mode with saved position/size
    glfwSetWindowMonitor(m_Window, nullptr, config.windowPosX, config.windowPosY,
                         config.windowedWidth, config.windowedHeight, 0);
    // Restore window decorations
    glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_TRUE);
    LOG_INFO("Switched to Windowed mode: " +
             std::to_string(config.windowedWidth) + "x" +
             std::to_string(config.windowedHeight));
    break;
  }

  case WindowMode::BORDERLESS_FULLSCREEN: {
    // Borderless fullscreen: windowed at monitor resolution with no decorations
    glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowMonitor(m_Window, nullptr, 0, 0, vidmode->width,
                         vidmode->height, 0);
    LOG_INFO("Switched to Borderless Fullscreen: " +
             std::to_string(vidmode->width) + "x" +
             std::to_string(vidmode->height));
    break;
  }

  case WindowMode::FULLSCREEN: {
    // True exclusive fullscreen
    glfwSetWindowMonitor(m_Window, monitor, 0, 0, vidmode->width,
                         vidmode->height, vidmode->refreshRate);
    LOG_INFO("Switched to Fullscreen: " + std::to_string(vidmode->width) +
             "x" + std::to_string(vidmode->height) + " @ " +
             std::to_string(vidmode->refreshRate) + "Hz");
    break;
  }
  }

  m_WindowMode = mode;
  config.windowMode = mode;

  // Update internal size (will also trigger resize callback via GLFW)
  glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
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

