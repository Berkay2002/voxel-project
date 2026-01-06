#pragma once

#include <vector>

namespace Core {

class VertexBuffer;

// Describes a single vertex attribute
struct VertexAttribute {
  unsigned int index;      // Attribute location in shader
  int size;               // Number of components (1, 2, 3, or 4)
  unsigned int type;      // GL_FLOAT, etc.
  bool normalized;
  int stride;             // Bytes between consecutive vertices
  size_t offset;          // Offset of this attribute in the vertex
};

class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  // Non-copyable
  VertexArray(const VertexArray &) = delete;
  VertexArray &operator=(const VertexArray &) = delete;

  // Movable
  VertexArray(VertexArray &&other) noexcept;
  VertexArray &operator=(VertexArray &&other) noexcept;

  void Bind() const;
  void Unbind() const;

  // Add a vertex buffer with its attribute layout
  void AddVertexBuffer(const VertexBuffer &vbo, 
                       const std::vector<VertexAttribute> &attributes);

  [[nodiscard]] unsigned int GetID() const { return m_ID; }

private:
  unsigned int m_ID = 0;
};

} // namespace Core
