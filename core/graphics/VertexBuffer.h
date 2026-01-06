#pragma once

#include <cstddef>

namespace Core {

class VertexBuffer {
public:
  VertexBuffer(const void *data, size_t size);
  ~VertexBuffer();

  // Non-copyable
  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;

  // Movable
  VertexBuffer(VertexBuffer &&other) noexcept;
  VertexBuffer &operator=(VertexBuffer &&other) noexcept;

  void Bind() const;
  void Unbind() const;

  [[nodiscard]] unsigned int GetID() const { return m_ID; }

private:
  unsigned int m_ID = 0;
};

} // namespace Core
