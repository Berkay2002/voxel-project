#include "VertexArray.h"
#include "VertexBuffer.h"

#include <glad/gl.h>

namespace Core {

VertexArray::VertexArray() {
  glGenVertexArrays(1, &m_ID);
}

VertexArray::~VertexArray() {
  if (m_ID != 0) {
    glDeleteVertexArrays(1, &m_ID);
  }
}

VertexArray::VertexArray(VertexArray &&other) noexcept : m_ID(other.m_ID) {
  other.m_ID = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&other) noexcept {
  if (this != &other) {
    if (m_ID != 0) {
      glDeleteVertexArrays(1, &m_ID);
    }
    m_ID = other.m_ID;
    other.m_ID = 0;
  }
  return *this;
}

void VertexArray::Bind() const {
  glBindVertexArray(m_ID);
}

void VertexArray::Unbind() const {
  glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const VertexBuffer &vbo,
                                   const std::vector<VertexAttribute> &attributes) {
  Bind();
  vbo.Bind();

  for (const auto &attr : attributes) {
    glEnableVertexAttribArray(attr.index);
    glVertexAttribPointer(
        attr.index,
        attr.size,
        attr.type,
        attr.normalized ? GL_TRUE : GL_FALSE,
        attr.stride,
        reinterpret_cast<const void *>(attr.offset)
    );
  }
}

} // namespace Core
