#ifndef SKYPE_HPP
#define SKYPE_HPP
#include "RenderableObject.hpp"
#include <glm/glm.hpp>

class CSkybox : public RenderableObject {
public:
  CSkybox();

  ~CSkybox() override;

  int GetTotalVertices() override;

  int GetTotalIndices() override;

  GLenum GetPrimitiveType() override;

  void FillVertexBuffer(GLfloat *pBuffer) override;

  void FillIndexBuffer(GLuint *pBuffer) override;
};
#endif //! SKYPE_HPP
