#include <cmath>
#include <iostream>

#include <GL/glew.h>

#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // output screen resolution
  static constexpr int WINDTH = 1280;
  static constexpr int HEIGHT = 960;

  // particle shader, textured shader and a pointer to current shader
  GLSLShader shader, texturedShader, *pCurrentShader;

  // IDs for vertex array and buffer object
  GLuint vaoID;
  GLuint vboVerticesID;

  // total number of particles
  static constexpr int MAX_PARTICLES = 10000;

  // projection modelview and emitter transform matrices
  glm::mat4 P            = glm::mat4(1);
  glm::mat4 MV           = glm::mat4(1);
  glm::mat4 emitterXForm = glm::mat4(1);

  // camera transformation variables
  int state   = 0, oldX = 0, oldY = 0;
  float rX    = 0, rY   = 0, dist = -10;
  float time_ = 0;

  // particle texture filename
  const std::string texture_filename = "media/particle.dds";

  // texture OpenGL texture ID
  GLuint textureID;
};
static Common *g_pCommon = nullptr;

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

// OpenGL initialization function
void OnInit() {
  GL_CHECK_ERRORS;
  // loader shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attribute and uniform
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("time");
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS;

  // load textured rendering shader
  g_pCommon->texturedShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  g_pCommon->texturedShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/textured.frag");
  // compile and link shader
  g_pCommon->texturedShader.CreateAndLinkProgram();
  g_pCommon->texturedShader.Use();
  // add attribute and uniform
  g_pCommon->texturedShader.AddUniform("MVP");
  g_pCommon->texturedShader.AddUniform("time");
  g_pCommon->texturedShader.AddUniform("textureMap");
  // set values of constant uniforms as initialization
  glUniform1i(g_pCommon->texturedShader("textureMap"), 0);
  g_pCommon->texturedShader.UnUse();

  GL_CHECK_ERRORS;

  // setup the vertex array object and vertex buffer object for the mesh
  // geometry handling
  glGenVertexArrays(1, &g_pCommon->vaoID);
  glBindVertexArray(g_pCommon->vaoID);

  // These calls marked start ATI/end ATI below are not required but on
  // ATI cards i tested, i had to pass an allocated buffer object
  // uncomment the code marked with start/end ATI if running on an ATI hardware
  /// start ATI ///
  /*
  glGenBuffers(1, &g_pCommon->vboVerticesID);
  glBindBuffer (GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
  glBufferData (GL_ARRAY_BUFFER, sizeof(GLubyte)*Common::MAX_PARTICLES, 0,
  GL_STATIC_DRAW); glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,1,GL_UNSIGNED_BYTE, GL_FALSE, 0,0);
  */
  /// end ATI ////

  GL_CHECK_ERRORS;

  // set particle size
  glPointSize(10);

  // enable blending and over blending operator
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // disable depth test
  glDisable(GL_DEPTH_TEST);

  // setup emitter xform
  g_pCommon->emitterXForm = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
  g_pCommon->emitterXForm = glm::rotate(g_pCommon->emitterXForm, 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));

  // load the texture
  int texture_width = 0, texture_height = 0, channels = 0;
  GLubyte *pData = SOIL_load_image(g_pCommon->texture_filename.c_str(), &texture_width,
                                   &texture_height, &channels, SOIL_LOAD_AUTO);
  if (pData == nullptr) {
    std::cerr << "Cannot load image: " << g_pCommon->texture_filename.c_str() << std::endl;
    exit(EXIT_FAILURE);
  }

  // Flip the image on Y axis
  int i, j;
  for (j = 0; j * 2 < texture_height; ++j) {
    int index1 = j * texture_width * channels;
    int index2 = (texture_height - 1 - j) * texture_width * channels;
    for (i = texture_width * channels; i > 0; --i) {
      GLubyte temp = pData[index1];
      pData[index1] = pData[index2];
      pData[index2] = temp;
      ++index1;
      ++index2;
    }
  }
  // get the image format
  GLenum format = GL_RGBA;
  switch (channels) {
  case 2:
    format = GL_RG32UI;
    break;
  case 3:
    format = GL_RGB;
    break;
  case 4:
    format = GL_RGBA;
    break;
  }

  // generate OpenGL texture
  glGenTextures(1, &g_pCommon->textureID);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->textureID);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // allocate texture
  glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), texture_width, texture_height, 0,
               format, GL_UNSIGNED_BYTE, pData);

  // release SOIL image data
  SOIL_free_image_data(pData);

  // set the particle shader as the current shader
  g_pCommon->pCurrentShader = &g_pCommon->shader;

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  // delete the particle texture
  glDeleteTextures(1, &g_pCommon->textureID);

  // Destroy shader
  g_pCommon->pCurrentShader = nullptr;
  g_pCommon->shader.DeleteShaderProgram();
  g_pCommon->texturedShader.DeleteShaderProgram();

  // Destroy vao and vbo
  /// start ATI ///
  // glDeleteBuffers(1, &g_pCommon->vboVerticesID);
  /// end ATI ///

  glDeleteVertexArrays(1, &g_pCommon->vaoID);

  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // setup the projection matrix
  g_pCommon->P = glm::perspective(60.0f, static_cast<float>(w) / h, 0.1f, 100.0f);
}

// display callback function
void OnRender() {
  // get current time
  GLfloat time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

  // clear colour and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transformation
  glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  glm::mat4 Rx = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 MV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  //glm::mat4 MVP = g_pCommon->P * MV;

  // bind the current shader
  g_pCommon->pCurrentShader->Use();
  // pass shader uniforms
  glUniform1f((*g_pCommon->pCurrentShader)("time"), time);
  glUniformMatrix4fv((*g_pCommon->pCurrentShader)("MVP"), 1, GL_FALSE,
                     glm::value_ptr(g_pCommon->P * MV * g_pCommon->emitterXForm));
  // render points
  glDrawArrays(GL_POINTS, 0, Common::MAX_PARTICLES);
  // unbind shader
  g_pCommon->pCurrentShader->UnUse();

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

// call display function when idle
void OnIdle() { glutPostRedisplay(); }

// keyboard event handler to toggle use of textured and coloured particle system
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
  case ' ':
    if (g_pCommon->pCurrentShader == &g_pCommon->shader) {
      g_pCommon->pCurrentShader = &g_pCommon->texturedShader;
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else {
      g_pCommon->pCurrentShader = &g_pCommon->shader;
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    break;
  }
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;

  // freeglut initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WINDTH, Common::HEIGHT);
  glutCreateWindow("Simple particles - OpenGL 3.3");

  // initialize glew
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
  GL_CHECK_ERRORS;

  // output hardware information
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;

  GL_CHECK_ERRORS;

  // OpenGL initialization
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutIdleFunc(OnIdle);
  glutKeyboardFunc(OnKey);

  // main loop call
  glutMainLoop();

  return 0;
}
