// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <string_view>

#include <GL/glew.h>
#include <SDL2/SDL.h>


struct Common {
  // Screen size
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;
  // Title
  static constexpr std::string_view window_title =
      std::string_view("Getting started with OpenGL 3.3");
  // SDL2 Window
  SDL_Window *m_pWindow = nullptr;
  // SDL2 OpenGL Context
  SDL_GLContext mGlContext;
};
static Common *g_pCommon = nullptr;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  Common common;
  g_pCommon = &common;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
    return -1;
  }
  g_pCommon->m_pWindow = SDL_CreateWindow(
      g_pCommon->window_title.data(), SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, Common::WIDTH, Common::HEIGHT, SDL_WINDOW_OPENGL);

  g_pCommon->mGlContext = SDL_GL_CreateContext(g_pCommon->m_pWindow);

  // Set our OpenGL version.
  // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions
  // are disabled
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // 3.3 is part of the modern versions of OpenGL, but most video cards whould
  // be able to run it
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // This makes our buffer swap syncronized with the monitor's vertical refresh
  SDL_GL_SetSwapInterval(1);

  // glew initialization
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
  } else {
    if (GLEW_VERSION_3_3) {
      std::cout << "Driver supports OpenGL 3.3\nDetails:" << std::endl;
    }
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
  while(loop) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        loop = false;
    }

    // clear colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // swap front and back buffers to show the rendered result
    SDL_GL_SwapWindow(g_pCommon->m_pWindow);
  }

	// Delete our OpengL context
	SDL_GL_DeleteContext(g_pCommon->mGlContext);
	// Destroy our window
	SDL_DestroyWindow(g_pCommon->m_pWindow);
	// Shutdown SDL 2
	SDL_Quit();

  return 0;
}
