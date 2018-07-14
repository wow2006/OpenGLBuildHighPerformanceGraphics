#include "Grid.hpp"
#include <glm/glm.hpp>

CGrid::CGrid(int _width, int _depth) {
  this->width = _width;
  this->depth = _depth;

  // setup shader
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
  shader.CreateAndLinkProgram();
  shader.Use();
  shader.AddAttribute("vVertex");
  shader.AddUniform("MVP");
  shader.UnUse();

  Init();
}

CGrid::~CGrid() {}

int CGrid::GetTotalVertices() { return ((width + 1) + (depth + 1)) * 2; }

int CGrid::GetTotalIndices() { return (width * depth); }

GLenum CGrid::GetPrimitiveType() { return GL_LINES; }

void CGrid::FillVertexBuffer(GLfloat *pBuffer) {
  glm::vec3 *vertices = reinterpret_cast<glm::vec3 *>(pBuffer);
  int count = 0;
  int width_2 = width / 2;
  int depth_2 = depth / 2;
  int i = 0;

  for (i = -width_2; i <= width_2; i++) {
    vertices[count++] = glm::vec3(i, 0, -depth_2);
    vertices[count++] = glm::vec3(i, 0, depth_2);

    vertices[count++] = glm::vec3(-width_2, 0, i);
    vertices[count++] = glm::vec3(width_2, 0, i);
  }
}

void CGrid::FillIndexBuffer(GLuint *pBuffer) {
  int i = 0;
  // fill indices array
  GLuint *id = pBuffer;
  for (i = 0; i < width * depth; i += 4) {
    *id++ = static_cast<GLuint>(i);
    *id++ = static_cast<GLuint>(i + 1);
    *id++ = static_cast<GLuint>(i + 2);
    *id++ = static_cast<GLuint>(i + 3);
  }
}
