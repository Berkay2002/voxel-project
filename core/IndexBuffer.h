#pragma once

#include <cstddef>

namespace Core {

class IndexBuffer {
public:
  IndexBuffer(const unsigned int *data, size_t count);
  ~IndexBuffer();

  // Non-copyable
  IndexBuffer(const IndexBuffer &) = delete;
  IndexBuffer &operator=(const IndexBuffer &) = delete;

  // Movable
  IndexBuffer(IndexBuffer &&other) noexcept;
  IndexBuffer &operator=(IndexBuffer &&other) noexcept;

  void Bind() const;
  void Unbind() const;

  [[nodiscard]] unsigned int GetID() const { return m_ID; }
  [[nodiscard]] size_t GetCount() const { return m_Count; }

private:
  unsigned int m_ID = 0;
  size_t m_Count = 0;
};

} // namespace Core
