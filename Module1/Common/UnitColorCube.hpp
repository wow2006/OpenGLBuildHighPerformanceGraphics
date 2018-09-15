#pragma once
#include "RenderableObject.hpp"

class CUnitColorCube : public RenderableObject {
public:
  CUnitColorCube();
  virtual ~CUnitColorCube();

  int GetTotalVertices();
  int GetTotalIndices();
  GLenum GetPrimitiveType();

  void FillVertexBuffer(GLfloat *pBuffer);
  void FillIndexBuffer(GLuint *pBuffer);
};
