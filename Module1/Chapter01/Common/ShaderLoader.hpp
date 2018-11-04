#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP
#include <array>
#include <iostream>
#include <string_view>
#include <unordered_map>

#include "GL_Wrapper.hpp"


namespace common {
enum Attribute {
  vertex = 0,
  color
};

enum class ShaderType : int {
  Vertex_Shader   = 0,
  Geomtery_Shader = 1,
  Fragment_Shader = 2
};

class ShaderLoader {
public:
  ShaderLoader();

  ~ShaderLoader();

  bool compileShaderFromFile(std::string_view shaderFilePath, ShaderType type);

  bool compileShader(std::string_view shaderPath, ShaderType type);

  bool linkShader();

  void use() const;

  static void unUse();

  bool addAttribute(std::string_view attributeName);

  bool AddUniform(std::string_view uniformName);

  GLuint operator[](std::string_view id) const;

protected:
  std::array<GLuint, 3> m_vShaders = {0, 0, 0};

  std::unordered_map<std::string_view, GLuint> mUniforms;

  GLuint mProgram = 0;
};

inline void ShaderLoader::use() const {
  glUseProgram(mProgram);
}

inline void ShaderLoader::unUse() {
  glUseProgram(0);
}

inline GLuint ShaderLoader::operator[](std::string_view id) const {
  return static_cast<GLuint>(mUniforms.at(id));
}

} // namespace common
#endif //! SHADER_LOADER_HPP

