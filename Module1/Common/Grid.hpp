#pragma once
#include <cstdint>

#include "RenderableObject.hpp"

class CGrid final : public RenderableObject {
public:
  CGrid(uint32_t width = 10, uint32_t depth = 10);
  ~CGrid() override;

  int GetTotalVertices() override;
  int GetTotalIndices() override;
  GLenum GetPrimitiveType() override;

  void FillVertexBuffer(GLfloat *pBuffer) override;
  void FillIndexBuffer(GLuint *pBuffer) override;

private:
  uint32_t width, depth;
};
