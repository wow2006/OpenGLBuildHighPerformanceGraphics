// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <array>
#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "Skybox.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen size
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  // projection and modelview matrices
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // camera transformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = -3, rY = 65, dist = -7;

  // skybox object
  std::unique_ptr<CSkybox> skybox;

  // skybox texture ID
  GLuint skyboxTextureID;

  // skybox texture names
  std::array<const char *, 6> texture_names = {
      "media/ocean/posx.png", "media/ocean/negx.png", "media/ocean/posy.png",
      "media/ocean/negy.png", "media/ocean/posz.png", "media/ocean/negz.png"};
};
static Common *g_pCommon = nullptr;

// mouse click handler
void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    g_pCommon->oldX = x;
    g_pCommon->oldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON) {
    g_pCommon->state = 0;
  } else {
    g_pCommon->state = 1;
  }
}

// mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->state == 0) {
    g_pCommon->dist *= (1 + (y - g_pCommon->oldY) / 60.0f);
  } else {
    g_pCommon->rY += (x - g_pCommon->oldX) / 5.0f;
    g_pCommon->rX += (y - g_pCommon->oldY) / 5.0f;
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  // enable depth testing and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  GL_CHECK_ERRORS

  // generate a new Skybox
  g_pCommon->skybox = std::make_unique<CSkybox>();

  GL_CHECK_ERRORS

  // load skybox textures using SOIL
  int texture_widths[6];
  int texture_heights[6];
  int channels[6];
  GLubyte *pData[6];

  std::cout << "Loading skybox images: ..." << std::endl;
  for (int i = 0; i < 6; i++) {
    const auto texture_name =
        g_pCommon->texture_names[static_cast<std::size_t>(i)];
    std::cout << "\tLoading: " << texture_name << " ... ";
    pData[i] =
        SOIL_load_image(texture_name, &texture_widths[i], &texture_heights[i],
                        &channels[i], SOIL_LOAD_AUTO);
    std::cout << "done." << std::endl;
  }

  GL_CHECK_ERRORS

  // generate OpenGL texture
  glGenTextures(1, &g_pCommon->skyboxTextureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, g_pCommon->skyboxTextureID);

  GL_CHECK_ERRORS
  // set texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  GL_CHECK_ERRORS

  // set the image format
  const auto format =
      static_cast<GLenum>((channels[0] == 4) ? GL_RGBA : GL_RGB);
  const auto internalFormat = static_cast<GLint>(format);
  // load the 6 images
  for (int i = 0; i < 6; i++) {
    // allocate cubemap data
    const auto index = static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
    glTexImage2D(index, 0, internalFormat, texture_widths[i],
                 texture_heights[i], 0, format, GL_UNSIGNED_BYTE, pData[i]);

    // free SOIL image data
    SOIL_free_image_data(pData[i]);
  }

  GL_CHECK_ERRORS
  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  g_pCommon->skybox.reset();

  glDeleteTextures(1, &g_pCommon->skyboxTextureID);
  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // setup the projection matrix
  g_pCommon->P = glm::perspective(
      45.0f, static_cast<GLfloat>(w) / static_cast<GLfloat>(h), 0.1f, 1000.f);
}

// idle event handler
void OnIdle() { glutPostRedisplay(); }

// display callback function
void OnRender() {
  GL_CHECK_ERRORS

  // clear colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  // const auto T   = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f,
  // g_pCommon->dist));
  const auto Rx =
      glm::rotate(glm::mat4(1), g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  const auto MV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  const auto S = glm::scale(glm::mat4(1), glm::vec3(1000.0));
  const auto MVP = g_pCommon->P * MV * S;

  // render the skybox object
  g_pCommon->skybox->Render(glm::value_ptr(MVP));

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;
  // freeglut initialization calls
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Skybox - OpenGL 3.3");

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
  err = glGetError(); // this is to ignore INVALID ENUM error 1282
  GL_CHECK_ERRORS

  // print information on screen
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;

  GL_CHECK_ERRORS

  // initialization of OpenGL
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutIdleFunc(OnIdle);

  // call main loop
  glutMainLoop();

  return 0;
}
