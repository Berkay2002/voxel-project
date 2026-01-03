#include "IndexBuffer.h"

#include <glad/gl.h>

namespace Core {

IndexBuffer::IndexBuffer(const unsigned int *data, size_t count) : m_Count(count) {
  glGenBuffers(1, &m_ID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
               static_cast<GLsizeiptr>(count * sizeof(unsigned int)), 
               data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer() {
  if (m_ID != 0) {
    glDeleteBuffers(1, &m_ID);
  }
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept 
    : m_ID(other.m_ID), m_Count(other.m_Count) {
  other.m_ID = 0;
  other.m_Count = 0;
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) noexcept {
  if (this != &other) {
    if (m_ID != 0) {
      glDeleteBuffers(1, &m_ID);
    }
    m_ID = other.m_ID;
    m_Count = other.m_Count;
    other.m_ID = 0;
    other.m_Count = 0;
  }
  return *this;
}

void IndexBuffer::Bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

void IndexBuffer::Unbind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace Core
