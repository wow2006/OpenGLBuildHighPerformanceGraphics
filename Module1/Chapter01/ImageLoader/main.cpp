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
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

namespace {
constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;
}    // namespace

struct Common {
  // shader reference
  GLSLShader shader;

  // vertex array and vertex buffer object IDs
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  // texture ID
  GLuint textureID;

  // quad vertices and indices
  glm::vec2 vertices[4];
  GLushort indices[6];

  // projection and modelview matrices
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // texture image filename
  const std::string filename = "media/Lenna.png";
};
static Common* g_pCommon = nullptr;

// OpenGL initialization
void OnInit() {
  GL_CHECK_ERRORS

  // load shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/imageLoader.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/imageLoader.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attributes and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddUniform("textureMap");
  // pass values of constant uniforms at initialization
  glUniform1i(g_pCommon->shader("textureMap"), 0);
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS

  // setup quad geometry
  // setup quad vertices
  g_pCommon->vertices[0] = glm::vec2(0.0, 0.0);
  g_pCommon->vertices[1] = glm::vec2(1.0, 0.0);
  g_pCommon->vertices[2] = glm::vec2(1.0, 1.0);
  g_pCommon->vertices[3] = glm::vec2(0.0, 1.0);

  // fill quad indices array
  GLushort* id = &g_pCommon->indices[0];
  *id++ = 0;
  *id++ = 1;
  *id++ = 2;
  *id++ = 0;
  *id++ = 2;
  *id++ = 3;

  GL_CHECK_ERRORS

  // setup quad vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->vaoID);
  glGenBuffers(1, &g_pCommon->vboVerticesID);
  glGenBuffers(1, &g_pCommon->vboIndicesID);

  glBindVertexArray(g_pCommon->vaoID);
  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
  // pass quad vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->vertices), &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
  glVertexAttribPointer(g_pCommon->shader["vVertex"], 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS
  // pass quad indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->indices), &g_pCommon->indices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS

  // load the image using SOIL
  int texture_width = 0, texture_height = 0, channels = 0;
  GLubyte* pData = SOIL_load_image(g_pCommon->filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
  if(!pData) {
    std::cerr << "Cannot load image: " << g_pCommon->filename.c_str() << std::endl;
    exit(EXIT_FAILURE);
  }
  // vertically flip the image on Y axis since it is inverted
  int i, j;
  for(j = 0; j * 2 < texture_height; ++j) {
    int index1 = j * texture_width * channels;
    int index2 = (texture_height - 1 - j) * texture_width * channels;
    for(i = texture_width * channels; i > 0; --i) {
      GLubyte temp = pData[index1];
      pData[index1] = pData[index2];
      pData[index2] = temp;
      ++index1;
      ++index2;
    }
  }
  // setup OpenGL texture and bind to texture unit 0
  glGenTextures(1, &g_pCommon->textureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->textureID);
  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // allocate texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);
  // free SOIL image data
  SOIL_free_image_data(pData);

  GL_CHECK_ERRORS

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  // Destroy shader
  g_pCommon->shader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1, &g_pCommon->vboVerticesID);
  glDeleteBuffers(1, &g_pCommon->vboIndicesID);
  glDeleteVertexArrays(1, &g_pCommon->vaoID);

  // Delete textures
  glDeleteTextures(1, &g_pCommon->textureID);
  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}

// display function
void OnRender() {
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // bind shader
  g_pCommon->shader.Use();
  // draw the full screen quad
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  // unbind shader
  g_pCommon->shader.UnUse();

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
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

  auto* window = glfwCreateWindow(WIDTH, HEIGHT, "Getting started with OpenGL 3.3", nullptr, nullptr);
  if(nullptr == window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);

  glbinding::initialize(glfwGetProcAddress);

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

  return EXIT_SUCCESS;
}
