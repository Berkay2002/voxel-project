#include "VertexBuffer.h"

#include <glad/gl.h>

namespace Core {

VertexBuffer::VertexBuffer(const void *data, size_t size) {
  glGenBuffers(1, &m_ID);
  glBindBuffer(GL_ARRAY_BUFFER, m_ID);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
  if (m_ID != 0) {
    glDeleteBuffers(1, &m_ID);
  }
}

VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept : m_ID(other.m_ID) {
  other.m_ID = 0;
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) noexcept {
  if (this != &other) {
    if (m_ID != 0) {
      glDeleteBuffers(1, &m_ID);
    }
    m_ID = other.m_ID;
    other.m_ID = 0;
  }
  return *this;
}

void VertexBuffer::Bind() const {
  glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void VertexBuffer::Unbind() const {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace Core
