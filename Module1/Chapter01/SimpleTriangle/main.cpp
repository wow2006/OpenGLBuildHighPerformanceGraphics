// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <string_view>

#include "common.hpp"
#include "ShaderLoader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


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

  /// Initialize vertices
  // setup triangle geometry
  // setup triangle vertices
  std::array<Vertex, 3> vertices = {
    Vertex{{-1.f,-1.f, 0.f},
           { 1.f, 0.f, 0.f}},
    Vertex{{ 0.f, 1.f, 0.f},
           { 0.f, 1.f, 0.f}},
    Vertex{{ 1.f,-1.f, 0.f},
           { 0.f, 0.f, 1.f}}
  };

  // setup triangle indices
  std::array<GLushort, 3> indices = {
    0, 1, 2
  };

  common::Model simpleTriangle;
  simpleTriangle.initlize(vertices.data(), sizeof(vertices),
                          indices.data(),  sizeof(indices));

  std::cout << "Initialization successfull" << std::endl;

  // projection and modelview matrices
  const glm::mat4 P  = glm::mat4(1);
  const glm::mat4 MV = glm::mat4(1);

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
    // draw triangle
    simpleTriangle.draw();
    // unbind the shader
    shader.unUse();

    // swap front and back buffers to show the rendered result
    SDL_GL_SwapWindow(common.m_pWindow);
  }

  return 0;
}
