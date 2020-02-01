// STL
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
// GLEW
#include <GL/glew.h>
// freeglut
#include <GL/freeglut.h>
// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
// Internal
#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen size
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  float frameTime;
  float currentTime;

  std::uint32_t totalFrames = 0;
  std::uint32_t startTime;
  float fps;
  float frameTimeQP;
  float delta_time;

  static constexpr auto INFO_SIZE = 1024;
  char info[INFO_SIZE];

  float dist;
  float rX, rY;
  glm::mat4 mMV, mP, mMVP;

  // Objects
  GLuint query, t_query;
  GLuint vaoUpdateID[2], vaoRenderID[2];
  GLuint gridVAOID, gridVBOVerticesID, gridVBOIndicesID;
  GLuint vboID_Pos[2], vboID_PrePos[2], vboID_Direction[2];
  GLuint tfID;

  GLSLShader renderShader;
  GLSLShader particleShader;
  GLSLShader passShader;
};
Common *g_pCommon = nullptr;

void OnShutdown() {
  glDeleteQueries(1, &g_pCommon->query);
  glDeleteQueries(1, &g_pCommon->t_query);

  glDeleteVertexArrays(2, g_pCommon->vaoUpdateID);
  glDeleteVertexArrays(2, g_pCommon->vaoRenderID);

  glDeleteVertexArrays(1, &g_pCommon->gridVAOID);
  glDeleteBuffers(1,      &g_pCommon->gridVBOVerticesID);
  glDeleteBuffers(1,      &g_pCommon->gridVBOIndicesID);

  glDeleteBuffers(2, g_pCommon->vboID_Pos);
  glDeleteBuffers(2, g_pCommon->vboID_PrePos);
  glDeleteBuffers(2, g_pCommon->vboID_Direction);

  glDeleteTransformFeedbacks(1, &g_pCommon->tfID);
  g_pCommon->renderShader.DeleteShaderProgram();
  g_pCommon->particleShader.DeleteShaderProgram();
  g_pCommon->passShader.DeleteShaderProgram();

  printf("Shutdown successful.");
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  // get the perspective projection matrix
  g_pCommon->mP =
      glm::perspective(60.0f, static_cast<float>(w) / h, 1.0f, 100.f);
}

// display callback function
void OnRender() {
  GL_CHECK_ERRORS

  // timing related function calls
  const auto newTime = static_cast<float>(glutGet(GLUT_ELAPSED_TIME));
  g_pCommon->frameTime = newTime - g_pCommon->currentTime;
  g_pCommon->currentTime = newTime;

  // TODO(Hussein): Using high res. counter
  // QueryPerformanceCounter(t2);
  // compute and print the elapsed time in millisec
  // frameTimeQP = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
  // t1 = t2;
  // accumulator += frameTimeQP;

  ++g_pCommon->totalFrames;

  // FPS calculation code
  if ((newTime - g_pCommon->startTime) > 1000) {
    float elapsedTime = (newTime - g_pCommon->startTime);
    g_pCommon->fps = (g_pCommon->totalFrames / elapsedTime) * 1000;
    g_pCommon->startTime = newTime;
    g_pCommon->totalFrames = 0;
    snprintf(g_pCommon->info, Common::INFO_SIZE,
             "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): "
             "%3.3f, TF Time: %3.3f",
             g_pCommon->fps, g_pCommon->frameTime, g_pCommon->frameTimeQP,
             g_pCommon->delta_time);
  }

  glutSetWindowTitle(g_pCommon->info);
  // clear the colour and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  const auto T =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  const auto Rx = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  g_pCommon->mMV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  g_pCommon->mMVP = g_pCommon->mP * g_pCommon->mMV;

  // draw grid
  // DrawGrid();

  // update particles on GPU
  // UpdateParticlesGPU();

  // render particles
  // RenderParticles();

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

void OnIdle() { glutPostRedisplay(); }

void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    oldX = x;
    oldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON)
    state = 0;
  else
    state = 1;
}

void OnMouseMove(int x, int y) {
  if (state == 0)
    dist *= (1 + (y - oldY) / 60.0f);
  else {
    rY += (x - oldX) / 5.0f;
    rX += (y - oldY) / 5.0f;
  }

  oldX = x;
  oldY = y;
  glutPostRedisplay();
}

auto main(int argc, char **argv) -> int {
  Common common;
  g_pCommon = &common;

  // freeglut initialization calls
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(g_pCommon->WIDTH, g_pCommon->HEIGHT);
  glutCreateWindow("Simple triangle - OpenGL 3.3");

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

  // opengl initialization
  // OnInit();

  // callback hooks
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnReshape);
  glutIdleFunc(OnIdle);

  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutCloseFunc(OnShutdown);

  // initialization of OpenGL
  InitGL();

  // main loop call
  glutMainLoop();

  return EXIT_SUCCESS;
}
