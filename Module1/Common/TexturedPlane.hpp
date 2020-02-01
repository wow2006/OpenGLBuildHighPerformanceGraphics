#pragma once
#include "RenderableObject.hpp"

class CTexturedPlane : public RenderableObject {
public:
  CTexturedPlane(const int width = 1000, const int depth = 1000);

  ~CTexturedPlane() override;

  int GetTotalVertices() override;

  int GetTotalIndices() override;

  GLenum GetPrimitiveType() override;

  void FillVertexBuffer(GLfloat *pBuffer) override;

  void FillIndexBuffer(GLuint *pBuffer) override;

private:
  int width, depth;
};
