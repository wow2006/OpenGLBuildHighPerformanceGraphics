#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Grid.hpp"
#include "Quad.hpp"
#include "GLSLShader.hpp"
#include "UnitColorCube.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  CGrid *m_pGrid = nullptr;

  CUnitColorCube *m_pCube = nullptr;

  CQuad *m_pMirror = nullptr;

  glm::mat4 P  = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // camera transformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 25, rY = -40, dist = -7;

  // FBO and render buffer object ID
  GLuint fboID, rbID;

  // offscreen render texture ID
  GLuint renderTextureID;

  // local rotation matrix
  glm::mat4 localR = glm::mat4(1);

  // autorotate angle
  float angle = 0;
};
static Common *g_pCommon = nullptr;

// initialize FBO
void InitFBO() {
  // generate and bind fbo ID
  glGenFramebuffers(1, &g_pCommon->fboID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_pCommon->fboID);

  // generate and bind render buffer ID
  glGenRenderbuffers(1, &g_pCommon->rbID);
  glBindRenderbuffer(GL_RENDERBUFFER, g_pCommon->rbID);

  // set the render buffer storage
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, Common::WIDTH, Common::HEIGHT);

  // generate the offscreen texture
  glGenTextures(1, &g_pCommon->renderTextureID);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->renderTextureID);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Common::WIDTH, Common::HEIGHT, 0, GL_BGRA,
               GL_UNSIGNED_BYTE, nullptr);

  // bind the renderTextureID as colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, g_pCommon->renderTextureID, 0);
  // set the render buffer as the depth attachment of FBO
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, g_pCommon->rbID);

  // check for frame buffer completeness status
  GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

  if (status == GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO setup succeeded.");
  } else {
    printf("Error in FBO setup.");
  }

  // unbind the texture and FBO
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShutdownFBO() {
  glDeleteTextures(1, &g_pCommon->renderTextureID);
  glDeleteRenderbuffers(1, &g_pCommon->rbID);
  glDeleteFramebuffers(1, &g_pCommon->fboID);
}

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

// mouse move event handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->state == 0)
    g_pCommon->dist *= (1 + (y - g_pCommon->oldY) / 60.0f);
  else {
    g_pCommon->rY += (x - g_pCommon->oldX) / 5.0f;
    g_pCommon->rX += (y - g_pCommon->oldY) / 5.0f;
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  glEnable(GL_DEPTH_TEST);

  GL_CHECK_ERRORS

  // create the 20x20 grid in XZ plane
  g_pCommon->m_pGrid = new CGrid(20, 20);

  // create a unit colour cube
  g_pCommon->m_pCube = new CUnitColorCube();

  // create a quad as mirror object at Z=-2 position
  g_pCommon->m_pMirror = new CQuad(-2,
                                   "shaders/Mirror/quad_shader.vert",
                                   "shaders/Mirror/quad_shader.frag");

  // initialize FBO object
  InitFBO();

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  ShutdownFBO();

  delete g_pCommon->m_pGrid;
  delete g_pCommon->m_pCube;
  delete g_pCommon->m_pMirror;

  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // setup the projection matrix
  g_pCommon->P = glm::perspective(45.0f, static_cast<GLfloat>(w) / h, 1.f, 1000.f);
}

// idle event handler
void OnIdle() {
  // increment angle and create a local rotation matrix on Y axis
  g_pCommon->angle += 0.5f;
  g_pCommon->localR = glm::rotate(glm::mat4(1), g_pCommon->angle, glm::vec3(0, 1, 0));

  // call the display function
  glutPostRedisplay();
}

// display callback
void OnRender() {
  // set the camera transformation
  glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  glm::mat4 Rx = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 Ry = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 MV = Ry;
  glm::mat4 MVP = g_pCommon->P * MV;

  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // render scene normally
  // render the grid
  g_pCommon->m_pGrid->Render(glm::value_ptr(MVP));
  g_pCommon->localR[3][1] = 0.5;

  // move the unit cube on Y axis to bring it to ground level
  // and render the cube
  g_pCommon->m_pCube->Render(glm::value_ptr(g_pCommon->P * MV * g_pCommon->localR));

  // store the current modelview matrix
  glm::mat4 oldMV = MV;

  // now change the view matrix to where the mirror is
  // reflect the view vector in the mirror normal direction
  glm::vec3 V = glm::vec3(-MV[2][0], -MV[2][1], -MV[2][2]);
  glm::vec3 R = glm::reflect(V, g_pCommon->m_pMirror->normal);

  // place the virtual camera at the mirro position
  MV = glm::lookAt(g_pCommon->m_pMirror->position, g_pCommon->m_pMirror->position + R, glm::vec3(0, 1, 0));

  // since mirror image is laterally inverted, we multiply the MV matrix by
  // (-1,1,1)
  MV = glm::scale(MV, glm::vec3(-1, 1, 1));

  // enable FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_pCommon->fboID);
  // render to colour attachment 0
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // show the mirror from the front side only
  if (glm::dot(V, g_pCommon->m_pMirror->normal) < 0) {
    g_pCommon->m_pGrid->Render(glm::value_ptr(g_pCommon->P * MV));
    g_pCommon->m_pCube->Render(glm::value_ptr(g_pCommon->P * MV * g_pCommon->localR));
  }

  // unbind the FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // restore the default back buffer
  glDrawBuffer(GL_BACK_LEFT);

  // reset the old modelview matrix
  MV = oldMV;

  // bind the FBO output at the current texture
  glBindTexture(GL_TEXTURE_2D, g_pCommon->renderTextureID);

  // render mirror
  g_pCommon->m_pMirror->Render(glm::value_ptr(g_pCommon->P * MV));

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Mirror using FBO - OpenGL 3.3");

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
  glutIdleFunc(OnIdle);

  // call main loop
  glutMainLoop();

  return 0;
}
