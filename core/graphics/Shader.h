#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace Core {

class Shader {
public:
  Shader(const std::string &vertexPath, const std::string &fragmentPath);
  ~Shader();

  // Non-copyable
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  // Movable
  Shader(Shader &&other) noexcept;
  Shader &operator=(Shader &&other) noexcept;

  void Bind() const;
  void Unbind() const;

  // Uniform setters
  void SetInt(const std::string &name, int value);
  void SetBool(const std::string &name, bool value);
  void SetFloat(const std::string &name, float value);
  void SetVec2(const std::string &name, const glm::vec2 &value);
  void SetVec3(const std::string &name, const glm::vec3 &value);
  void SetVec4(const std::string &name, const glm::vec4 &value);
  void SetMat4(const std::string &name, const glm::mat4 &value);

  [[nodiscard]] unsigned int GetID() const { return m_ID; }
  [[nodiscard]] bool IsValid() const { return m_ID != 0; }

private:
  unsigned int m_ID = 0;
  
  // Cache uniform locations to avoid repeated glGetUniformLocation calls
  mutable std::unordered_map<std::string, int> m_UniformLocationCache;

  static std::string ReadFile(const std::string &path);
  static unsigned int CompileShader(unsigned int type, const std::string &source);
  static unsigned int CreateProgram(unsigned int vertexShader, unsigned int fragmentShader);
  [[nodiscard]] int GetUniformLocation(const std::string &name);
};

} // namespace Core
