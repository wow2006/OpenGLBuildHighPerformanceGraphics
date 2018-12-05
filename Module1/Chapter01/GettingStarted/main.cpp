// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>

#include "common.hpp"


int main(int argc, char **argv) {
  common::Common common;

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

  glClearColor(1, 0, 0, 1);
  std::cout << "Initialization successfull" << std::endl;

  bool loop = true;
  while (loop) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        loop = false;
      }
    }

    // clear colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // swap front and back buffers to show the rendered result
    SDL_GL_SwapWindow(common.m_pWindow);
  }

  return 0;
}
