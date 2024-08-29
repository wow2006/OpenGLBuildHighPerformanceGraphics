// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <cstdio>
#include <cstdlib>

#include <fmt/color.h>
#include <fmt/printf.h>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"

using namespace gl;


namespace {
constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;
constexpr auto TITLE = "Simple triangle - OpenGL 3.3";
}    // namespace

// OpenGL initialization
std::tuple<GLuint, GLuint, GLuint> createBuffers(GLSLShader& shader) {
  // vertex array and vertex buffer object IDs
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  // out vertex struct for interleaved attributes
  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
  };

  // triangle vertices and indices
  Vertex vertices[3];
  GLushort indices[3];


  // setup triangle geometry
  // setup triangle vertices
  vertices[0].color = glm::vec3(1, 0, 0);
  vertices[1].color = glm::vec3(0, 1, 0);
  vertices[2].color = glm::vec3(0, 0, 1);

  vertices[0].position = glm::vec3(-1, -1, 0);
  vertices[1].position = glm::vec3(0, 1, 0);
  vertices[2].position = glm::vec3(1, -1, 0);

  // setup triangle indices
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;

  // setup triangle vao and vbo stuff
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);
  GLsizei stride = sizeof(Vertex);

  glBindVertexArray(vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass triangle verteices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

  // enable vertex attribute array for position
  glEnableVertexAttribArray(shader["vVertex"]);
  glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE, stride, nullptr);

  // enable vertex attribute array for colour
  glEnableVertexAttribArray(shader["vColor"]);
  glVertexAttribPointer(shader["vColor"], 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offsetof(Vertex, color)));

  // pass indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

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

  auto* window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
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
        if(severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
          fmt::print(stderr, fg(fmt::color::red), "OpenGL ERROR: {}\n", message);
        }
      },
      nullptr);

  // shader reference
  GLSLShader shader;

  // projection and modelview matrices
  glm::mat4 P = glm::ortho(-1, 1, -1, 1);
  ;
  glm::mat4 MV = glm::mat4(1);

  glfwSetWindowSizeCallback(window, [](GLFWwindow*, int width, int height) {
    // set the viewport size
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  });

  // load the shader
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
  // compile and link shader
  shader.CreateAndLinkProgram();
  shader.Use();
  // add attributes and uniforms
  shader.AddAttribute("vVertex");
  shader.AddAttribute("vColor");
  shader.AddUniform("MVP");
  shader.UnUse();

  const auto [vaoID, vboVerticesID, vboIndicesID] = createBuffers(shader);

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // clear the colour and depth buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // bind the shader
    shader.Use();
    // pass the shader uniform
    glUniformMatrix4fv(static_cast<int>(shader("MVP")), 1, GL_FALSE, glm::value_ptr(P * MV));
    // drwa triangle
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
    // unbind the shader
    shader.UnUse();

    glfwSwapBuffers(window);
  }

  // Destroy shader
  shader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1, &vboVerticesID);
  glDeleteBuffers(1, &vboIndicesID);
  glDeleteVertexArrays(1, &vaoID);

  fmt::print("Shutdown successfull\n");

  return 0;
}
