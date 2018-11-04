// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <string_view>

#include "common.hpp"
#include "ShaderLoader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};


int main(int argc, char **argv) {
  common::Common common;
  common.window_title = "Simple triangle - OpenGL 3.3";

  auto args = gsl::make_span(argv, argc);
  common.parseArgments(std::move(args));

  if(!common.initialize()) {
    return -1;
  }
  
  // print information on screen
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;

  common::ShaderLoader shader;
  // load the shader
  shader.compileShaderFromFile("shaders/shader.vert", common::ShaderType::Vertex_Shader);
  shader.compileShaderFromFile("shaders/shader.frag", common::ShaderType::Fragment_Shader);
  // compile and link shader
  shader.linkShader();
  shader.use();
  // add attributes and uniforms
  shader.AddUniform("MVP");

  std::array<Vertex,   3> vertices;
  std::array<GLushort, 3> indices;

  /// Initialize vertices
  // setup triangle geometry
  // setup triangle vertices
  vertices[0].color = glm::vec3(1, 0, 0);
  vertices[1].color = glm::vec3(0, 1, 0);
  vertices[2].color = glm::vec3(0, 0, 1);

  vertices[0].position = glm::vec3(-1, -1, 0);
  vertices[1].position = glm::vec3(0,   1, 0);
  vertices[2].position = glm::vec3(1,  -1, 0);

  // setup triangle indices
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;

  GLuint vaoID = 0, vboVerticesID = 0, vboIndicesID = 0;
  // setup triangle vao and vbo stuff
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);
  GLsizei stride = sizeof(Vertex);

  glBindVertexArray(vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass triangle verteices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
               &vertices[0], GL_STATIC_DRAW);

  // enable vertex attribute array for position
  glEnableVertexAttribArray(common::vertex);
  glVertexAttribPointer(common::vertex, 3, GL_FLOAT, GL_FALSE,
                        stride, nullptr);

  // enable vertex attribute array for colour
  glEnableVertexAttribArray(common::color);
  glVertexAttribPointer(common::color, 3, GL_FLOAT, GL_FALSE, stride,
      OFFSET(offsetof(Vertex, color)));

  // pass indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
               &indices[0], GL_STATIC_DRAW);

  std::cout << "Initialization successfull" << std::endl;

  // projection and modelview matrices
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  bool loop = true;
  while (loop) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        loop = false;
      }
    }

    // clear colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT);

    // bind the shader
    shader.use();
    // pass the shader uniform
    glUniformMatrix4fv(shader["MVP"], 1, GL_FALSE,
                       glm::value_ptr(P * MV));
    // drwa triangle
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
    // unbind the shader
    shader.unUse();

    // swap front and back buffers to show the rendered result
    SDL_GL_SwapWindow(common.m_pWindow);
  }

  return 0;
}
