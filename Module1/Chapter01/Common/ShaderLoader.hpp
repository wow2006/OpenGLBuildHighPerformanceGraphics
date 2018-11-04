#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP
#include <array>
#include <iostream>
#include <string_view>
#include <unordered_map>

#include "GL_Wrapper.hpp"


namespace common {
enum class ShaderType : int {
  Vertex_Shader   = 0,
  Geomtery_Shader = 1,
  Fragment_Shader = 2
};

static GLenum type_to_enum(ShaderType type) {
  switch(type) {
  case ShaderType::Vertex_Shader:
    return GL_VERTEX_SHADER;
  case ShaderType::Geomtery_Shader:
   return GL_GEOMETRY_SHADER;
  case ShaderType::Fragment_Shader:
    return GL_FRAGMENT_SHADER;
  }
}

class ShaderLoader {
public:
  ShaderLoader();

  ~ShaderLoader();

  bool compileShader(std::string_view shaderPath, ShaderType type);

  bool linkShader();

  void use() const;

  static void unUse();

  bool addAttribute(std::string_view attributeName);

  bool AddUniform(std::string_view uniformName);

protected:
  std::array<GLuint, static_cast<int>(ShaderType::Fragment_Shader)+1> m_vShaders;

  std::unordered_map<std::string_view, GLuint> mUniforms;

  GLuint mProgram = 0;
};

inline void ShaderLoader::use() const {
  glUseProgram(mProgram);
}

inline void ShaderLoader::unUse() {
  glUseProgram(0);
}

} // namespace common
#endif //! SHADER_LOADER_HPP

