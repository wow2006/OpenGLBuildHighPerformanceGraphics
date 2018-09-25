#pragma once
#include "RenderableObject.hpp"
#include <glm/glm.hpp>


class CQuad : public RenderableObject {
public:
  CQuad(float zpos = 0,
        const char* vert = "shaders/quad_shader.vert",
        const char* frag = "shaders/quad_shader.frag");
  virtual ~CQuad();

  int GetTotalVertices();
  int GetTotalIndices();
  GLenum GetPrimitiveType();

  void FillVertexBuffer(GLfloat *pBuffer);
  void FillIndexBuffer(GLuint *pBuffer);

  float Zpos;
  glm::vec3 position;
  glm::vec3 normal;
};
