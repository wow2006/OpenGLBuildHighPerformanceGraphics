// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "GLSLShader.hpp"
#include <iostream>

GLSLShader::GLSLShader() {
  _shaders[VERTEX_SHADER]   = 0;
  _shaders[FRAGMENT_SHADER] = 0;
  _shaders[GEOMETRY_SHADER] = 0;
  _attributeList.clear();
  _uniformLocationList.clear();
}

GLSLShader::~GLSLShader() {
  _attributeList.clear();
  _uniformLocationList.clear();
}

void GLSLShader::DeleteShaderProgram() { glDeleteProgram(_program); }

void GLSLShader::LoadFromString(GLenum type, const std::string &source) {
  GLuint shader = glCreateShader(type);

  const char *ptmp = source.c_str();
  glShaderSource(shader, 1, &ptmp, nullptr);

  // check whether the shader loads fine
  GLint status;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar *infoLog = new GLchar[static_cast<std::size_t>(infoLogLength)];
    glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog);
    std::cerr << "Compile log: " << infoLog << std::endl;
    delete[] infoLog;
  }
  _shaders[_totalShaders++] = shader;
}

void GLSLShader::CreateAndLinkProgram() {
  _program = glCreateProgram();
  if (_shaders[VERTEX_SHADER] != 0) {
    glAttachShader(_program, _shaders[VERTEX_SHADER]);
  }
  if (_shaders[FRAGMENT_SHADER] != 0) {
    glAttachShader(_program, _shaders[FRAGMENT_SHADER]);
  }
  if (_shaders[GEOMETRY_SHADER] != 0) {
    glAttachShader(_program, _shaders[GEOMETRY_SHADER]);
  }

  // link and check whether the program links fine
  GLint status;
  glLinkProgram(_program);
  glGetProgramiv(_program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLogLength;

    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar *infoLog = new GLchar[static_cast<std::size_t>(infoLogLength)];
    glGetProgramInfoLog(_program, infoLogLength, nullptr, infoLog);
    std::cerr << "Link log: " << infoLog << std::endl;
    delete[] infoLog;
  }

  glDeleteShader(_shaders[VERTEX_SHADER]);
  glDeleteShader(_shaders[FRAGMENT_SHADER]);
  glDeleteShader(_shaders[GEOMETRY_SHADER]);
}

void GLSLShader::Use() { glUseProgram(_program); }

void GLSLShader::UnUse() { glUseProgram(0); }

void GLSLShader::AddAttribute(const std::string &attribute) {
  _attributeList[attribute] =
      static_cast<GLuint>(glGetAttribLocation(_program, attribute.c_str()));
}

// An indexer that returns the location of the attribute
GLuint GLSLShader::operator[](const std::string &attribute) {
  return _attributeList[attribute];
}

void GLSLShader::AddUniform(const std::string &uniform) {
  _uniformLocationList[uniform] =
      static_cast<GLuint>(glGetUniformLocation(_program, uniform.c_str()));
}

GLuint GLSLShader::operator()(const std::string &uniform) {
  return _uniformLocationList[uniform];
}

#include <fstream>
void GLSLShader::LoadFromFile(GLenum whichShader, const std::string &filename) {
  std::ifstream fp;
  fp.open(filename.c_str(), std::ios_base::in);
  if (fp) {
    std::string line, buffer;
    while (getline(fp, line)) {
      buffer.append(line);
      buffer.append("\r\n");
    }
    // copy to source
    LoadFromString(whichShader, buffer);
  } else {
    std::cerr << "Error loading shader: " << filename << std::endl;
  }
}
