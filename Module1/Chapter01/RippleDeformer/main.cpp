// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  //screen size
  const int WIDTH  = 1280;
  const int HEIGHT = 960;

  //shader reference
  GLSLShader shader;

  //vertex array and vertex buffer object IDs
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  static constexpr int NUM_X = 40; // total quads on X axis
  static constexpr int NUM_Z = 40; // total quads on Z axis

  const float SIZE_X = 4; // size of plane in world space
  const float SIZE_Z = 4;
  const float HALF_SIZE_X = SIZE_X / 2.0f;
  const float HALF_SIZE_Z = SIZE_Z / 2.0f;

  // ripple displacement speed
  const float SPEED = 2;

  //ripple mesh vertices and indices
  glm::vec3 vertices[(NUM_X+1)*(NUM_Z+1)];
  static constexpr int TOTAL_INDICES = NUM_X*NUM_Z*2*3;
  GLushort indices[TOTAL_INDICES];

  // projection and modelview matrices
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // camera transformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 25, rY = -40, dist = -7;

  // current time
  float time = 0;
};
static Common* g_pCommon = nullptr;

// mosue click handler
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

// mosue move handler
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
  // set the polygon mode to render lines
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  GL_CHECK_ERRORS
  // load shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/RippleDeformer.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/RippleDeformer.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add shader attribute and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("time");
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS

  // setup plane geometry
  // setup plane vertices
  int count = 0;
  int i = 0, j = 0;
  for (j = 0; j <= g_pCommon->NUM_Z; j++) {
    for (i = 0; i <= g_pCommon->NUM_X; i++) {
      g_pCommon->vertices[count++] =
          glm::vec3(((float(i) / (g_pCommon->NUM_X - 1)) * 2 - 1) * g_pCommon->HALF_SIZE_X, 0,
                    ((float(j) / (g_pCommon->NUM_Z - 1)) * 2 - 1) * g_pCommon->HALF_SIZE_Z);
    }
  }

  // fill plane indices array
  GLushort *id = &g_pCommon->indices[0];
  for (i = 0; i < Common::NUM_Z; i++) {
    for (j = 0; j < Common::NUM_X; j++) {
      int i0 = i * (Common::NUM_X + 1) + j;
      int i1 = i0 + 1;
      int i2 = i0 + (Common::NUM_X + 1);
      int i3 = i2 + 1;
      if ((j + i) % 2) {
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

  GL_CHECK_ERRORS

  // setup plane vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->vaoID);
  glGenBuffers(1, &g_pCommon->vboVerticesID);
  glGenBuffers(1, &g_pCommon->vboIndicesID);

  glBindVertexArray(g_pCommon->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
  // pass plane vertices to array buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->vertices), &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attrib array for position
  glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
  glVertexAttribPointer(g_pCommon->shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS
  // pass the plane indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->indices), &g_pCommon->indices[0],
               GL_STATIC_DRAW);
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

  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // setup the projection matrix
  g_pCommon->P = glm::perspective(45.0f, static_cast<GLfloat>(w) / h, 1.f, 1000.f);
}

// idle event callback
void OnIdle() { glutPostRedisplay(); }

// display callback
void OnRender() {
  // get the elapse time
  g_pCommon->time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f * g_pCommon->SPEED;

  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set teh camera viewing transformation
  glm::mat4 T   = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  glm::mat4 Rx  = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  g_pCommon->MV  = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 MVP = g_pCommon->P * g_pCommon->MV;

  // bind the shader
  g_pCommon->shader.Use();
  // set the shader uniforms
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  glUniform1f(g_pCommon->shader("time"), g_pCommon->time);
  // draw the mesh triangles
  glDrawElements(GL_TRIANGLES, g_pCommon->TOTAL_INDICES, GL_UNSIGNED_SHORT, nullptr);

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
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(g_pCommon->WIDTH, g_pCommon->HEIGHT);
  glutCreateWindow("Ripple deformer - OpenGL 3.3");

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
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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

  // main loop call
  glutMainLoop();

  return 0;
}
