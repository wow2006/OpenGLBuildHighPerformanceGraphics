// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <tuple>

#include <fmt/color.h>
#include <fmt/printf.h>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "GLSLShader.hpp"

using namespace gl;

namespace {
constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;
constexpr std::string_view TITLE = "Getting started with OpenGL 3.3";
}    // namespace

std::optional<GLuint> createTexture(std::string_view filename) {
  // load the image using SOIL
  int texture_width = 0, texture_height = 0, channels = 0;

  // vertically flip the image on Y axis since it is inverted
  stbi_set_flip_vertically_on_load(1);

  using TextureHandler = std::unique_ptr<GLubyte, decltype(&stbi_image_free)>;
  auto texturePtr = TextureHandler(stbi_load(filename.data(), &texture_width, &texture_height, &channels, 3), stbi_image_free);
  if(!texturePtr) {
    fmt::print(stderr, fg(fmt::color::red), "Cannot load image: {}\n", filename);
    return std::nullopt;
  }

  // setup OpenGL texture and bind to texture unit 0
  GLuint textureID = 0;
  glGenTextures(1, &textureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // allocate texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texturePtr.get());
  return textureID;
}

std::tuple<GLuint, GLuint, GLuint> createBuffers(GLSLShader& shader) {
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  // setup quad geometry
  // setup quad vertices
  constexpr std::array<glm::vec2, 4> vertices {glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 1.0)};

  // fill quad indices array
  constexpr std::array<GLushort, 6> indices {0, 1, 2, 0, 2, 3};

  // setup quad vao and vbo stuff
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);


  glBindVertexArray(vaoID);
  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass quad vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

  // enable vertex attribute array for position
  glEnableVertexAttribArray(shader["vVertex"]);
  glVertexAttribPointer(shader["vVertex"], 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // pass quad indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

  return {vaoID, vboVerticesID, vboIndicesID};
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
  glfwSetErrorCallback([](int errorCode, const char* message) {
    fmt::print(stderr, fg(fmt::color::red), "GLFW ERROR({}): {}\n", errorCode, message);
  });

  // glfw initialization
  if(GLFW_FALSE == glfwInit()) {
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_DEPTH_BITS, 16);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

  auto* window = glfwCreateWindow(WIDTH, HEIGHT, TITLE.data(), nullptr, nullptr);
  if(nullptr == window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);

  glbinding::initialize(glfwGetProcAddress);

  fmt::print("Driver supports OpenGL 3.3\nDetails:\n");
  {
    int major = 0;
    int minor = 0;
    int rev = 0;
    glfwGetVersion(&major, &minor, &rev);
    fmt::print("\tUsing GLEW {}.{}.{}\n", major, minor, rev);
  }
  fmt::print("\tVendor: {}\n", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
  fmt::print("\tRenderer: {}\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
  fmt::print("\tVersion: {}\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  fmt::print("\tGLSL: {}\n", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

  glDebugMessageCallbackARB(
      [](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const char* message, const void*) {
        if(GL_DEBUG_SEVERITY_HIGH_ARB == severity) {
          fmt::print(stderr, fg(fmt::color::red), "OpenGL ERROR: {}\n", message);
        }
      },
      nullptr);

  glfwSetWindowSizeCallback(window, [](GLFWwindow*, int width, int height) {
    // set the viewport size
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  });

  GLSLShader shader;
  // load shader
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/imageLoader.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/imageLoader.frag");
  // compile and link shader
  shader.CreateAndLinkProgram();
  shader.Use();
  // add attributes and uniforms
  shader.AddAttribute("vVertex");
  shader.AddUniform("textureMap");
  // pass values of constant uniforms at initialization
  glUniform1i(static_cast<int>(shader("textureMap")), 0);
  shader.UnUse();

  const auto [vaoID, vboVerticesID, vboIndicesID] = createBuffers(shader);

  // texture image filename
  const auto textureOpt = createTexture("media/Lenna.png");
  if(!textureOpt) {
    return EXIT_FAILURE;
  }
  const auto textureID = textureOpt.value();

  fmt::print("Initialization successfull\n");

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // clear the colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shader
    shader.Use();
    // draw the full screen quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    // unbind shader
    shader.UnUse();

    glfwSwapBuffers(window);
  }

  // Destroy shader
  shader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1, &vboVerticesID);
  glDeleteBuffers(1, &vboIndicesID);
  glDeleteVertexArrays(1, &vaoID);

  // Delete textures
  glDeleteTextures(1, &textureID);
  fmt::print("Shutdown successfull\n");

  return EXIT_SUCCESS;
}
