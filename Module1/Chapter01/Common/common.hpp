#ifndef COMMON_HPP
#define COMMON_HPP
#include <iostream>
#include <unordered_set>

#include <gsl/gsl>

#include "GL_Wrapper.hpp"
#include <SDL.h>

inline constexpr const char* errorToString(const uint error) {
  switch(error) {
    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";
    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";
    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";
    default:
      return "Unknown error";
  }
}

inline void checkOpenGL(const char* fileName, const char* function, const int lineNumber) {
  auto error = glGetError();
  if(error == GL_NO_ERROR) {
    return;
  }
  auto errorString = errorToString(error);
  printf("error %s:%d %s : [%d]%s", fileName, lineNumber, function, error, errorString);
}

#define GL_CHECK_ERRORS checkOpenGL(__FILE__, __PRETTY_FUNCTION__, __LINE__)

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