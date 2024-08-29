// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <array>
#include <cstdio>
#include <cstdlib>
#include <tuple>

#include <fmt/color.h>
#include <fmt/printf.h>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glm/trigonometric.hpp>
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
constexpr std::string_view TITLE = "Ripple deformer - OpenGL 3.3";
constexpr int NUM_X = 40;    // total quads on X axis
constexpr int NUM_Z = 40;    // total quads on Z axis

constexpr float SIZE_X = 4.0F;    // size of plane in world space
constexpr float SIZE_Z = 4.0F;
constexpr float HALF_SIZE_X = SIZE_X / 2.0F;
constexpr float HALF_SIZE_Z = SIZE_Z / 2.0F;

// ripple displacement speed
constexpr float SPEED = 2.0F;

constexpr int TOTAL_INDICES = NUM_X * NUM_Z * 2 * 3;
}    // namespace

std::tuple<GLuint, GLuint, GLuint> createBuffers(GLSLShader& shader) {
  // vertex array and vertex buffer object IDs
  GLuint vaoID = 0;
  GLuint vboVerticesID = 0;
  GLuint vboIndicesID = 0;

  // ripple mesh vertices and indices
  std::array<glm::vec3, (NUM_X + 1) * (NUM_Z + 1)> vertices;
  // setup plane geometry
  // setup plane vertices
  int count = 0;
  int i = 0, j = 0;
  for(j = 0; j <= NUM_Z; j++) {
    for(i = 0; i <= NUM_X; i++) {
      vertices[count++] = glm::vec3(
          ((float(i) / (NUM_X - 1)) * 2 - 1) * HALF_SIZE_X, 0, ((float(j) / (NUM_Z - 1)) * 2 - 1) * HALF_SIZE_Z);
    }
  }

  std::array<GLushort, TOTAL_INDICES> indices;
  // fill plane indices array
  GLushort* id = indices.data();
  for(i = 0; i < NUM_Z; i++) {
    for(j = 0; j < NUM_X; j++) {
      int i0 = i * (NUM_X + 1) + j;
      int i1 = i0 + 1;
      int i2 = i0 + (NUM_X + 1);
      int i3 = i2 + 1;
      if((j + i) % 2) {
        *id++ = static_cast<GLushort>(i0);
        *id++ = static_cast<GLushort>(i2);
        *id++ = static_cast<GLushort>(i1);
        *id++ = static_cast<GLushort>(i1);
        *id++ = static_cast<GLushort>(i2);
        *id++ = static_cast<GLushort>(i3);
      } else {
        *id++ = static_cast<GLushort>(i0);
        *id++ = static_cast<GLushort>(i2);
        *id++ = static_cast<GLushort>(i3);
        *id++ = static_cast<GLushort>(i0);
        *id++ = static_cast<GLushort>(i3);
        *id++ = static_cast<GLushort>(i1);
      }
    }
  }

  // setup plane vao and vbo stuff
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);

  glBindVertexArray(vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass plane vertices to array buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

  // enable vertex attrib array for position
  const auto vertexAttrib = shader["vVertex"];
  glEnableVertexAttribArray(vertexAttrib);
  glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // pass the plane indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(), GL_STATIC_DRAW);

  fmt::print("Initialization successfull\n");

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

  struct UserDefinedData {
    // projection and modelview matrices
    glm::mat4 P = glm::perspective(45.0F, static_cast<GLfloat>(WIDTH) / HEIGHT, 1.0F, 1000.0F);
    ;
    glm::mat4 MV = glm::mat4(1);

    // camera transformation variables
    int state = 0, oldX = 0, oldY = 0;
    float rX = 25.0F, rY = -40.0F, dist = -7.0F;

    // current time
    float time = 0;
    bool leftClicked = false;
  };
  UserDefinedData userDefinedData;

  glfwSetWindowUserPointer(window, &userDefinedData);
  glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    static auto* user = static_cast<UserDefinedData*>(glfwGetWindowUserPointer(window));
    // set the viewport size
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    // setup the projection matrix
    user->P = glm::perspective(45.0F, static_cast<GLfloat>(width) / height, 1.0F, 1000.0F);
  });
  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int) {
    static auto* user = static_cast<UserDefinedData*>(glfwGetWindowUserPointer(window));
    if(GLFW_PRESS == action) {
      double x = 0, y = 0;
      glfwGetCursorPos(window, &x, &y);
      user->oldX = static_cast<int>(x);
      user->oldY = static_cast<int>(y);
      user->leftClicked = true;
    } else {
      user->leftClicked = false;
    }

    if(GLFW_MOUSE_BUTTON_MIDDLE == button) {
      user->state = 0;
    } else {
      user->state = 1;
    }
  });
  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
    static auto* user = static_cast<UserDefinedData*>(glfwGetWindowUserPointer(window));
    if(user->leftClicked) {
      fmt::print("\r(rX, rY) = ({}, {}), ({}, {}), {}", user->rX, user->rY, user->oldX, user->oldY, user->dist);
      if(0 == user->state) {
        user->dist *= (1.0F + static_cast<float>(y - static_cast<double>(user->oldY)) / 60.0F);
      } else {
        user->rY += static_cast<float>(x - static_cast<double>(user->oldX)) / 5.0F;
        user->rX += static_cast<float>(y - static_cast<double>(user->oldY)) / 5.0F;
      }
      user->oldX = static_cast<int>(x);
      user->oldY = static_cast<int>(y);
    }
  });

  glfwSetCursorPos(window, static_cast<double>(WIDTH) / 2.0, static_cast<double>(HEIGHT) / 2.0);

  glDebugMessageCallbackARB(
      [](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const char* message, const void*) {
        if(GL_DEBUG_SEVERITY_HIGH_ARB == severity) {
          fmt::print(stderr, fg(fmt::color::red), "OpenGL ERROR: {}\n", message);
        }
      },
      nullptr);

  // load shader
  GLSLShader shader;
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/RippleDeformer.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/RippleDeformer.frag");
  // compile and link shader
  shader.CreateAndLinkProgram();
  shader.Use();
  // add shader attribute and uniforms
  shader.AddAttribute("vVertex");
  shader.AddUniform("MVP");
  shader.AddUniform("time");
  shader.UnUse();

  const auto [vaoID, vboVerticesID, vboIndicesID] = createBuffers(shader);

  // set the polygon mode to render lines
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // get the elapse time
    userDefinedData.time = static_cast<float>(glfwGetTime()) * SPEED;

    // clear the colour
    glClear(GL_COLOR_BUFFER_BIT);

    // set teh camera viewing transformation
    glm::mat4 T = glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, userDefinedData.dist));
    glm::mat4 Rx = glm::rotate(T, glm::radians(userDefinedData.rX), glm::vec3(1.0F, 0.0F, 0.0F));
    userDefinedData.MV = glm::rotate(Rx, glm::radians(userDefinedData.rY), glm::vec3(0.0F, 1.0F, 0.0F));
    glm::mat4 MVP = userDefinedData.P * userDefinedData.MV;

    // bind the shader
    shader.Use();
    // set the shader uniforms
    glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    glUniform1f(shader("time"), userDefinedData.time);
    // draw the mesh triangles
    glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_SHORT, nullptr);

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

  return EXIT_SUCCESS;
}
