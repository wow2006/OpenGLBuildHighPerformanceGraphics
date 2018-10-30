#ifndef COMMON_HPP
#define COMMON_HPP
#include <gsl/gsl>
#include <iostream>

#include <GL/glew.h>

#include <SDL2/SDL.h>

namespace common {
struct Common {
  Common();

  ~Common();

  // Screen size
  int mWidth  = 1280;
  int mHeight = 960;

  // Title
  std::string window_title = std::string("Getting started with OpenGL 3.3");

  // SDL2 Window
  SDL_Window *m_pWindow = nullptr;

  // SDL2 OpenGL Context
  SDL_GLContext mGlContext;

  bool initialize();

  void parseArgments(gsl::span<char *> &&args);
};
} // namespace common
#endif //! COMMON_HPP
