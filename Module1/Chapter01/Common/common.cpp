// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "common.hpp"

namespace common {

Common::Common() = default;

Common::~Common() {
  // Delete our OpengL context
  SDL_GL_DeleteContext(mGlContext);
  // Destroy our window
  SDL_DestroyWindow(m_pWindow);
  // Shutdown SDL 2
  SDL_Quit();
}

static void debugCallback(GLenum source, GLenum type, GLuint id,
                          GLenum severity, GLsizei length, const GLchar *msg,
                          const void *data) {
  (void)source;
  (void)type;
  (void)id;
  (void)severity;
  (void)length;
  (void)data;
  std::cout << "Debug: " << msg << std::endl;
}

bool Common::initialize() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
    return false;
  }

  m_pWindow = SDL_CreateWindow(window_title.c_str(), SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, mWidth, mHeight,
                               SDL_WINDOW_OPENGL);

  mGlContext = SDL_GL_CreateContext(m_pWindow);

  // Set our OpenGL version.
  // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions
  // are disabled
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

  // 3.3 is part of the modern versions of OpenGL, but most video cards whould
  // be able to run it
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // This makes our buffer swap syncronized with the monitor's vertical
  // refresh
  SDL_GL_SetSwapInterval(1);

  glEnable(GL_DEBUG_OUTPUT);

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

  if (GLEW_KHR_debug) {
    std::cout << "DEBUG Inialized\n";
    glDebugMessageCallback(debugCallback, nullptr);
  }

  return true;
}

void Common::parseArgments(gsl::span<char *> &&args) {
  for (auto arg = args.cbegin(); arg != args.cend(); ++arg) {
    auto value = std::string_view(*arg);
    if (value.find("--width") != std::string_view::npos ||
        value.find("-w") != std::string_view::npos) {
      mWidth = std::stoi(*++arg);
    } else if (value.find("--height") != std::string_view::npos ||
               value.find("-h") != std::string_view::npos) {
      mHeight = std::stoi(*++arg);
    }
  }
}

} // namespace common
