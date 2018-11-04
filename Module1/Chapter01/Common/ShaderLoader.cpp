#include <iostream>
#include <sstream>
#include <fstream>
#include "ShaderLoader.hpp"


namespace common {
  ShaderLoader::ShaderLoader() = default;

  ShaderLoader::~ShaderLoader() = default;

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

  static std::string type_to_string(ShaderType type) {
    switch(type) {
      case ShaderType::Vertex_Shader:
        return "vertex shader";
      case ShaderType::Geomtery_Shader:
        return "geometry shader";
      case ShaderType::Fragment_Shader:
        return "fragment shader";
    }
  }

  bool ShaderLoader::compileShaderFromFile(std::string_view shaderFilePath, ShaderType type) {
    std::ifstream file{shaderFilePath.data()};
    if(file.is_open()) {
      std::stringstream buffer;
      std::string line;
      while(std::getline(file, line)) {
        buffer << line << '\n';
      }
      return compileShader(buffer.str(), type);
    }
    std::cerr << "Can not find " << shaderFilePath;
    return false;
  }

  bool ShaderLoader::compileShader(std::string_view shaderSource, ShaderType type) {
    auto shader_type = type_to_enum(type);
    auto shader      = glCreateShader(shader_type);

    auto ptmp = shaderSource.data();
    glShaderSource(shader, 1, &ptmp, nullptr);

    // check whether the shader loads fine
    GLint status;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
      GLint infoLogLength;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

      std::string infoLog;
      infoLog.resize(static_cast<std::size_t>(infoLogLength));
      glGetShaderInfoLog(shader, infoLogLength, nullptr, &infoLog[0]);
      std::cerr << type_to_string(type) << "\tCompile log: " << infoLog << std::endl;
      return false;
    }
    auto index = static_cast<decltype(m_vShaders)::size_type>(type);
    m_vShaders[index] = shader;
    return true;
  }

  bool ShaderLoader::linkShader() {
    mProgram = glCreateProgram();
    for(auto shader : m_vShaders) {
      if(shader) {
        glAttachShader(mProgram, shader);
      }
    }

    // link and check whether the program links fine
    GLint status;
    glLinkProgram(mProgram);
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
      GLint infoLogLength;

      glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
      std::string infoLog;
      infoLog.resize(static_cast<size_t>(infoLogLength));
      glGetProgramInfoLog(mProgram, infoLogLength, nullptr, &infoLog[0]);
      std::cerr << "Link log: " << infoLog << std::endl;
      return false;
    }

    for(auto& shader : m_vShaders) {
      glDeleteShader(shader);
    }
    return true;
  }

  bool ShaderLoader::AddUniform(std::string_view uniformName) {
    if(!mProgram) {
      std::cerr << "Create program first\n";
      return false;
    }

    auto uniform = glGetUniformLocation(mProgram, uniformName.data());
    if(uniform >= 0 ) {
      mUniforms[uniformName] = static_cast<GLuint>(uniform);
      return true;
    }

    std::cerr << "Can not find uniform "
      << uniformName << " : " << uniform << '\n';
    return false;
  }

} // namespace common
