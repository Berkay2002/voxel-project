#pragma once

#include <string>

namespace Core {

class Texture {
public:
  explicit Texture(const std::string &path);
  ~Texture();

  // Non-copyable
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  // Movable
  Texture(Texture &&other) noexcept;
  Texture &operator=(Texture &&other) noexcept;

  void Bind(unsigned int slot = 0) const;
  void Unbind() const;

  [[nodiscard]] unsigned int GetID() const { return m_ID; }
  [[nodiscard]] int GetWidth() const { return m_Width; }
  [[nodiscard]] int GetHeight() const { return m_Height; }
  [[nodiscard]] bool IsValid() const { return m_ID != 0; }

private:
  unsigned int m_ID = 0;
  int m_Width = 0;
  int m_Height = 0;
  int m_Channels = 0;
};

} // namespace Core
