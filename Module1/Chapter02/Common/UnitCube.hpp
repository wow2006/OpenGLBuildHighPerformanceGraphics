#pragma once
#include "RenderableObject.hpp"
#include <glm/glm.hpp>

class CUnitCube : public RenderableObject {
public:
  CUnitCube(const glm::vec3 &color = glm::vec3(1, 1, 1));

  ~CUnitCube() override;

  CUnitCube(const CUnitCube&);

  CUnitCube(CUnitCube&&);

  CUnitCube& operator=(const CUnitCube&);

  CUnitCube& operator=(CUnitCube&&);

  int GetTotalVertices() override;
  int GetTotalIndices() override;
  GLenum GetPrimitiveType() override;
  void SetCustomUniforms() override;

  void FillVertexBuffer(GLfloat *pBuffer) override;
  void FillIndexBuffer(GLuint *pBuffer) override;

  glm::vec3 color;
};
