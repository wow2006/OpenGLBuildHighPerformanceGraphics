// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Grid.hpp"

#include <glm/glm.hpp>

namespace {
constexpr auto VertexShader = R"(#version 330 core

layout(location=0) in vec3 vVertex; //object space position

//uniform
uniform mat4 MVP;	//combined modelview projection

void main()
{
	//multiply the combined MVP matrix with the object space position to get the clip space position
	gl_Position = MVP*vec4(vVertex.xyz,1);
})";

constexpr auto FragmentShader = R"(#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment output colour

void main()
{
	//output constant white colour vec4(1,1,1,1)
	vFragColor = vec4(1,1,1,1);
})";
}

CGrid::CGrid(uint32_t _width, uint32_t _depth) : width{_width}, depth{_depth} {
  // setup shader
  shader.LoadFromString(GL_VERTEX_SHADER,   VertexShader);
  shader.LoadFromString(GL_FRAGMENT_SHADER, FragmentShader);
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
