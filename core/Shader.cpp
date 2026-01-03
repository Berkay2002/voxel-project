#include "Shader.h"
#include "Logger.h"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace Core {

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) {
  std::string vertexSource = ReadFile(vertexPath);
  std::string fragmentSource = ReadFile(fragmentPath);

  if (vertexSource.empty() || fragmentSource.empty()) {
    LogError("Failed to read shader files");
    return;
  }

  unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
  unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

  if (vertexShader == 0 || fragmentShader == 0) {
    if (vertexShader != 0) glDeleteShader(vertexShader);
    if (fragmentShader != 0) glDeleteShader(fragmentShader);
    return;
  }

  m_ID = CreateProgram(vertexShader, fragmentShader);

  // Shaders are linked into program, no longer needed
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  if (m_ID != 0) {
    LogInfo("Shader program created successfully");
  }
}

Shader::~Shader() {
  if (m_ID != 0) {
    glDeleteProgram(m_ID);
  }
}

Shader::Shader(Shader &&other) noexcept : m_ID(other.m_ID) {
  other.m_ID = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    if (m_ID != 0) {
      glDeleteProgram(m_ID);
    }
    m_ID = other.m_ID;
    other.m_ID = 0;
  }
  return *this;
}

void Shader::Bind() const {
  glUseProgram(m_ID);
}

void Shader::Unbind() const {
  glUseProgram(0);
}

void Shader::SetInt(const std::string &name, int value) const {
  glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string &name, float value) const {
  glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec3(const std::string &name, const glm::vec3 &value) const {
  glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string &name, const glm::vec4 &value) const {
  glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string &name, const glm::mat4 &value) const {
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

std::string Shader::ReadFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    LogError("Failed to open shader file: " + path);
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string &source) {
  unsigned int shader = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::string shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    LogError(shaderType + " shader compilation failed: " + infoLog);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

unsigned int Shader::CreateProgram(unsigned int vertexShader, unsigned int fragmentShader) {
  unsigned int program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  int success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, nullptr, infoLog);
    LogError("Shader program linking failed: " + std::string(infoLog));
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

int Shader::GetUniformLocation(const std::string &name) const {
  int location = glGetUniformLocation(m_ID, name.c_str());
  if (location == -1) {
    LogWarn("Uniform '" + name + "' not found in shader");
  }
  return location;
}

} // namespace Core
