#pragma once
#include <map>
#include <string>

#include <GL/glew.h>

class GLSLShader {
public:
  GLSLShader();
  ~GLSLShader();
  void LoadFromString(GLenum whichShader, const std::string &source);
  void LoadFromFile(GLenum whichShader, const std::string &filename);
  void CreateAndLinkProgram();
  void Use();
  void UnUse();
  void AddAttribute(const std::string &attribute);
  void AddUniform(const std::string &uniform);

  // An indexer that returns the location of the attribute/uniform
  GLuint operator[](const std::string &attribute);
  GLuint operator()(const std::string &uniform);
  void DeleteShaderProgram();

private:
  enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER };
  GLuint _program;
  int _totalShaders;
  GLuint _shaders[3]; // 0->vertexshader, 1->fragmentshader, 2->geometryshader
  std::map<std::string, GLuint> _attributeList;
  std::map<std::string, GLuint> _uniformLocationList;
};
