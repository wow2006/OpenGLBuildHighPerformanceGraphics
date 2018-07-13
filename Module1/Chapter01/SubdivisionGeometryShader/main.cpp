#include <iostream>
#include <algorithm>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen size
  const int WIDTH = 1280;
  const int HEIGHT = 960;

  // shader
  GLSLShader shader;

  // vertex array and vertex buffer object ids
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  // mesh vertices and indices
  glm::vec3 vertices[4];
  GLushort  indices[6];

  // projection and modelview matrices
  glm::mat4 P  = glm::mat4(1);
  //glm::mat4 MV = glm::mat4(1);

  // camera transformation variables
  int state = 0,  oldX = 0,   oldY   = 0;
  float rX  = 25, rY   = -40, dist   = -35;

  // number of sub-divisions
  int sub_divisions = 1;
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

// key event handler to increase/decrease number of sub-divisions
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
  case ',':
    g_pCommon->sub_divisions--;
    break;
  case '.':
    g_pCommon->sub_divisions++;
    break;
  }

  g_pCommon->sub_divisions = std::max<int>(1, std::min<int>(8, g_pCommon->sub_divisions));

  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  GL_CHECK_ERRORS
  // load the shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER,   "shaders/subdivisionGeometryShader.vert");
  g_pCommon->shader.LoadFromFile(GL_GEOMETRY_SHADER, "shaders/subdivisionGeometryShader.geom");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/subdivisionGeometryShader.frag");
  // create and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attribute and uniform
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("sub_divisions");
  // set values of constant uniforms at initialization
  glUniform1i(g_pCommon->shader("sub_divisions"), g_pCommon->sub_divisions);
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS

  // setup quad geometry
  // setup quad vertices
  g_pCommon->vertices[0] = glm::vec3(-5, 0, -5);
  g_pCommon->vertices[1] = glm::vec3(-5, 0, 5);
  g_pCommon->vertices[2] = glm::vec3(5,  0, 5);
  g_pCommon->vertices[3] = glm::vec3(5,  0, -5);

  // setup quad indices
  GLushort *id = &g_pCommon->indices[0];
  *id++ = 0;
  *id++ = 1;
  *id++ = 2;

  *id++ = 0;
  *id++ = 2;
  *id++ = 3;

  GL_CHECK_ERRORS

  // setup quad vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->vaoID);
  glGenBuffers(1,      &g_pCommon->vboVerticesID);
  glGenBuffers(1,      &g_pCommon->vboIndicesID);

  glBindVertexArray(g_pCommon->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
  // pass the quad vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->vertices), &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
  glVertexAttribPointer(g_pCommon->shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS
  // pass the quad indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->indices), &g_pCommon->indices[0],
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS

  // set the polygon mode to render lines
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  GL_CHECK_ERRORS

  std::cout << "Initialization successfull" << std::endl;
}

// delete all allocated resources
void OnShutdown() {
  // Destroy shader
  g_pCommon->shader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1,      &g_pCommon->vboVerticesID);
  glDeleteBuffers(1,      &g_pCommon->vboIndicesID);
  glDeleteVertexArrays(1, &g_pCommon->vaoID);

  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  // setup the projection matrix
  g_pCommon->P = glm::perspective(45.0f, static_cast<GLfloat>(w) / h, 0.01f, 10000.f);
}

// display callback function
void OnRender() {
  // clear colour and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transformation
  glm::mat4 T  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  glm::mat4 Rx = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 MV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  MV = glm::translate(MV, glm::vec3(-5, 0, -5));

  // bind the shader
  g_pCommon->shader.Use();
  // set the shader uniforms
  glUniform1i(g_pCommon->shader("sub_divisions"), g_pCommon->sub_divisions);
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(g_pCommon->P * MV));
  // draw the first submesh
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

  MV = glm::translate(MV, glm::vec3(10, 0, 0));
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(g_pCommon->P * MV));
  // draw the second submesh
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

  MV = glm::translate(MV, glm::vec3(0, 0, 10));
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(g_pCommon->P * MV));
  // draw the third submesh
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

  MV = glm::translate(MV, glm::vec3(-10, 0, 0));
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(g_pCommon->P * MV));
  // draw the fourth submesh
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  // unbind the shader
  g_pCommon->shader.UnUse();

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
  glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
//glutInitContextFlags();
  glutInitWindowSize(g_pCommon->WIDTH, g_pCommon->HEIGHT);
  glutCreateWindow(
      "Simple plane subdivision using geometry shader - OpenGL 3.3");

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
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)      << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)    << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)     << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;

  GL_CHECK_ERRORS

  // opengl initialization
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutKeyboardFunc(OnKey);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);

  // call main loop
  glutMainLoop();

  return 0;
}
