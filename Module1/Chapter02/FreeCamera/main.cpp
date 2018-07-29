// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <sstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FreeCamera.hpp"
#include "GLSLShader.hpp"
#include "TexturedPlane.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen size
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  // for floating point imprecision
  static constexpr float EPSILON = 0.001f;
  static constexpr float EPSILON2 = EPSILON * EPSILON;

  // camera tranformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 0, rY = 0, fov = 45;

  // delta time
  float dt = 0;

  // timing related variables
  float last_time = 0, current_time = 0;

  // free camera instance
  CFreeCamera cam;

  // mouse filtering support variables
  static constexpr float MOUSE_FILTER_WEIGHT = 0.75f;
  static constexpr int MOUSE_HISTORY_BUFFER_SIZE = 10;

  // mouse history buffer
  glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

  float mouseX = 0, mouseY = 0; // filtered mouse values

  // flag to enable filtering
  bool useFiltering = true;

  // output message
  std::stringstream msg;

  // floor checker texture ID
  GLuint checkerTextureID;

  // checkered plane object
  CTexturedPlane *checker_plane;
};
static Common *g_pCommon = nullptr;

// mouse move filtering function
void filterMouseMoves(float dx, float dy) {
  for (int i = Common::MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
    g_pCommon->mouseHistory[i] = g_pCommon->mouseHistory[i - 1];
  }

  // Store current mouse entry at front of array.
  g_pCommon->mouseHistory[0] = glm::vec2(dx, dy);

  float averageX = 0.0f;
  float averageY = 0.0f;
  float averageTotal = 0.0f;
  float currentWeight = 1.0f;

  // Filter the mouse.
  for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
    glm::vec2 tmp = g_pCommon->mouseHistory[i];
    averageX += tmp.x * currentWeight;
    averageY += tmp.y * currentWeight;
    averageTotal += 1.0f * currentWeight;
    currentWeight *= Common::MOUSE_FILTER_WEIGHT;
  }

  g_pCommon->mouseX = averageX / averageTotal;
  g_pCommon->mouseY = averageY / averageTotal;
}

// mouse click handler
void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    g_pCommon->oldX = x;
    g_pCommon->oldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON)
    g_pCommon->state = 0;
  else
    g_pCommon->state = 1;
}

// mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->state == 0) {
    g_pCommon->fov += (y - g_pCommon->oldY) / 5.0f;
    g_pCommon->cam.SetupProjection(g_pCommon->fov, g_pCommon->cam.GetAspectRatio());
  } else {
    g_pCommon->rY += (y - g_pCommon->oldY) / 5.0f;
    g_pCommon->rX += (g_pCommon->oldX - x) / 5.0f;
    if (g_pCommon->useFiltering)
      filterMouseMoves(g_pCommon->rX, g_pCommon->rY);
    else {
      g_pCommon->mouseX = g_pCommon->rX;
      g_pCommon->mouseY = g_pCommon->rY;
    }
    g_pCommon->cam.Rotate(g_pCommon->mouseX, g_pCommon->mouseY, 0);
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  glutPostRedisplay();
}

// initialize OpenGL
void OnInit() {
  GL_CHECK_ERRORS
  // generate the checker texture
  GLubyte data[128][128] = {{0}};
  for (int j = 0; j < 128; j++) {
    for (int i = 0; i < 128; i++) {
      data[i][j] = (i <= 64 && j <= 64) || (i > 64 && j > 64) ? 255 : 0;
    }
  }
  // generate texture object
  glGenTextures(1, &g_pCommon->checkerTextureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->checkerTextureID);
  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GL_CHECK_ERRORS

  // set maximum aniostropy setting
  GLfloat largest_supported_anisotropy;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                  largest_supported_anisotropy);

  // set mipmap base and max level
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

  // allocate texture object
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE,
               data);

  // generate mipmaps
  glGenerateMipmap(GL_TEXTURE_2D);

  GL_CHECK_ERRORS

  // create a textured plane object
  g_pCommon->checker_plane = new CTexturedPlane();

  GL_CHECK_ERRORS

  // setup camera
  // setup the camera position and look direction
  glm::vec3 p = glm::vec3(5);
  g_pCommon->cam.SetPosition(p);
  glm::vec3 look = glm::normalize(p);

  // rotate the camera for proper orientation
  float yaw = glm::degrees(std::atan2(look.z, look.x) + static_cast<float>(M_PI));
  float pitch = glm::degrees(std::asin(look.y));
  g_pCommon->rX = yaw;
  g_pCommon->rY = pitch;
  if (g_pCommon->useFiltering) {
    for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
      g_pCommon->mouseHistory[i] = glm::vec2(g_pCommon->rX, g_pCommon->rY);
    }
  }
  g_pCommon->cam.Rotate(g_pCommon->rX, g_pCommon->rY, 0);
  std::cout << "Initialization successfull" << std::endl;
}

// delete all allocated resources
void OnShutdown() {
  delete g_pCommon->checker_plane;
  glDeleteTextures(1, &g_pCommon->checkerTextureID);
  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // setup the camera projection matrix
  g_pCommon->cam.SetupProjection(45, static_cast<GLsizei>(w) / h);
}

// idle event processing
void OnIdle() {
  glm::vec3 t = g_pCommon->cam.GetTranslation();
  if (glm::dot(t, t) > Common::EPSILON2) {
    g_pCommon->cam.SetTranslation(t * 0.95f);
  }

  // call the display function
  glutPostRedisplay();
}

// display callback function
void OnRender() {
  // timing related calcualtion
  g_pCommon->last_time = g_pCommon->current_time;
  g_pCommon->current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
  g_pCommon->dt = g_pCommon->current_time - g_pCommon->last_time;

  // clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transformation
  glm::mat4 MV = g_pCommon->cam.GetViewMatrix();
  glm::mat4 P = g_pCommon->cam.GetProjectionMatrix();
  glm::mat4 MVP = P * MV;

  // render the chekered plane
  g_pCommon->checker_plane->Render(glm::value_ptr(MVP));

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

// Keyboard event handler to toggle the mouse filtering using spacebar key
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
  case ' ':
    g_pCommon->useFiltering = !g_pCommon->useFiltering;
    break;
  case 'w':
    g_pCommon->cam.Walk(g_pCommon->dt);
    break;
  case 's':
    g_pCommon->cam.Walk(-g_pCommon->dt);
    break;
  case 'a':
    g_pCommon->cam.Strafe(-g_pCommon->dt);
    break;
  case 'd':
    g_pCommon->cam.Strafe(g_pCommon->dt);
    break;
  case 'q':
    g_pCommon->cam.Lift(g_pCommon->dt);
    break;
  case 'z':
    g_pCommon->cam.Lift(-g_pCommon->dt);
    break;
  }
  glutPostRedisplay();
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
  glutCreateWindow("Free Camera - OpenGL 3.3");

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
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  GL_CHECK_ERRORS

  // opengl initialization
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutKeyboardFunc(OnKey);
  glutIdleFunc(OnIdle);

  // call main loop
  glutMainLoop();

  return 0;
}
