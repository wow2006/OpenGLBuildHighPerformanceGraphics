// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <fmt/color.h>
#include <fmt/format.h>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "GLSLShader.hpp"

using namespace gl;

namespace {
constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;
constexpr auto TITLE = "Simple plane subdivision using geometry shader - OpenGL 3.3";
}    // namespace

std::tuple<GLuint, GLuint, GLuint> createBuffers(GLSLShader& shader) {
  // vertex array and vertex buffer object IDs
  GLuint vaoID = 0;
  GLuint vboVerticesID = 0;
  GLuint vboIndicesID = 0;

  // mesh vertices and indices
  glm::vec3 vertices[4];
  GLushort indices[6];

  // setup quad geometry
  // setup quad vertices
  vertices[0] = glm::vec3(-5, 0, -5);
  vertices[1] = glm::vec3(-5, 0, 5);
  vertices[2] = glm::vec3(5, 0, 5);
  vertices[3] = glm::vec3(5, 0, -5);

  // setup quad indices
  GLushort* id = &indices[0];
  *id++ = 0;
  *id++ = 1;
  *id++ = 2;

  *id++ = 0;
  *id++ = 2;
  *id++ = 3;

  // setup quad vao and vbo stuff
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);

  glBindVertexArray(vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass the quad vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

  // enable vertex attribute array for position
  glEnableVertexAttribArray(shader["vVertex"]);
  glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  // pass the quad indices to element array buffer
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

  // print information on screen
  fmt::print("Driver supports OpenGL 3.3\nDetails:\n");
  {
    int major = 0;
    int minor = 0;
    int rev = 0;
    glfwGetVersion(&major, &minor, &rev);
    fmt::print("\tUsing GLFW {}.{}.{}\n", major, minor, rev);
  }
  fmt::print("\tVendor:    {}\n", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
  fmt::print("\tRenderer:  {}\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
  fmt::print("\tVersion:   {}\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  fmt::print("\tGLSL:      {}\n", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

  struct UserDefinedData {
    // projection and modelview matrices
    glm::mat4 P = glm::perspective(45.0F, static_cast<GLfloat>(WIDTH) / HEIGHT, 0.01F, 10000.0F);;

    // camera transformation variables
    int state = 0, oldX = 0, oldY = 0;
    float rX = 25, rY = -40, dist = -35;

    // number of sub-divisions
    int sub_divisions = 1;

    bool leftClicked = false;
  };
  UserDefinedData userDefinedData;

  glfwSetWindowUserPointer(window, &userDefinedData);
  glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    static auto* user = static_cast<UserDefinedData*>(glfwGetWindowUserPointer(window));
    // set the viewport size
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    // setup the projection matrix
    user->P = glm::perspective(45.0F, static_cast<GLfloat>(width) / height, 0.01F, 10000.0F);
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
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int, int action, int) {
    static auto* user = static_cast<UserDefinedData*>(glfwGetWindowUserPointer(window));
    if(GLFW_PRESS == action) {
      if(GLFW_KEY_COMMA == key) {
        user->sub_divisions--;
      }
      if(GLFW_KEY_PERIOD == key) {
        user->sub_divisions++;
      }
      user->sub_divisions = std::max<int>(1, std::min<int>(8, user->sub_divisions));
    }
  });

  glfwSetCursorPos(window, static_cast<double>(WIDTH) / 2.0, static_cast<double>(HEIGHT) / 2.0);

  glDebugMessageCallbackARB(
      [](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const char* message, const void*) {
        if(severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
          fmt::print(stderr, fg(fmt::color::red), "OpenGL ERROR: {}\n", message);
        }
      },
      nullptr);

  GLSLShader shader;
  // load the shader
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/subdivisionGeometryShader.vert");
  shader.LoadFromFile(GL_GEOMETRY_SHADER, "shaders/subdivisionGeometryShader.geom");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/subdivisionGeometryShader.frag");
  // create and link shader
  shader.CreateAndLinkProgram();
  shader.Use();
  // add attribute and uniform
  shader.AddAttribute("vVertex");
  shader.AddUniform("MVP");
  shader.AddUniform("sub_divisions");
  // set values of constant uniforms at initialization
  glUniform1i(shader("sub_divisions"), userDefinedData.sub_divisions);
  shader.UnUse();

  auto [vaoID, vboVerticesID, vboIndicesID] = createBuffers(shader);

  fmt::print("Initialization successfull\n");

  // set the polygon mode to render lines
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // clear colour and depth buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // set the camera transformation
    glm::mat4 T = glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, userDefinedData.dist));
    glm::mat4 Rx = glm::rotate(T, glm::radians(userDefinedData.rX), glm::vec3(1.0F, 0.0F, 0.0F));
    glm::mat4 MV = glm::rotate(Rx, glm::radians(userDefinedData.rY), glm::vec3(0.0F, 1.0F, 0.0F));
    MV = glm::translate(MV, glm::vec3(-5, 0, -5));

    // bind the shader
    shader.Use();
    // set the shader uniforms
    glUniform1i(shader("sub_divisions"), userDefinedData.sub_divisions);
    glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(userDefinedData.P * MV));
    // draw the first submesh
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    MV = glm::translate(MV, glm::vec3(10, 0, 0));
    glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(userDefinedData.P * MV));
    // draw the second submesh
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    MV = glm::translate(MV, glm::vec3(0, 0, 10));
    glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(userDefinedData.P * MV));
    // draw the third submesh
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    MV = glm::translate(MV, glm::vec3(-10, 0, 0));
    glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(userDefinedData.P * MV));
    // draw the fourth submesh
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
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
