// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <sstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FreeCamera.hpp"
#include "GLSLShader.hpp"
#include "Grid.hpp"
#include "UnitCube.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen size
  static constexpr int WIDTH  = 1280;
  static constexpr int HEIGHT = 960;

  // camera tranformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 0, rY = 0, fov = 45;

  // for floating point imprecision
  static constexpr float EPSILON = 0.001f;
  static constexpr float EPSILON2 = EPSILON * EPSILON;

  // delta time
  float dt = 0;

  // time related variables
  float current_time = 0, last_time = 0;

  // free camera instance
  CFreeCamera cam;

  // grid object
  CGrid *grid;

  // unit cube object
  CUnitCube *cube;

  // modelview and projection matrices
  glm::mat4 MV, P;

  // selected box index
  int selected_box = -1;

  // box positions
  glm::vec3 box_positions[3] = {glm::vec3(-1, 0.5, 0), glm::vec3(0, 0.5, 1),
                                glm::vec3(1, 0.5, 0)};

  // output message
  std::stringstream msg;

  // mouse filtering variables
  static constexpr float MOUSE_FILTER_WEIGHT = 0.75f;
  static constexpr int MOUSE_HISTORY_BUFFER_SIZE = 10;

  // mouse history buffer
  glm::vec2 mouseHistory[Common::MOUSE_HISTORY_BUFFER_SIZE];

  float mouseX = 0, mouseY = 0; // filtered mouse values

  // flag to enable filtering
  bool useFiltering = true;
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

    float winZ = 0;
    // read pixel depth at mouse click position
    glReadPixels(x, Common::HEIGHT - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    // unproject the obtained winx,winy and winz point to get the object space
    // point
    glm::vec3 objPt = glm::unProject(glm::vec3(x, Common::HEIGHT - y, winZ),
                                     g_pCommon->MV, g_pCommon->P,
                                     glm::vec4(0, 0, Common::WIDTH, Common::HEIGHT));

    size_t i = 0;
    float minDist = 1000;
    g_pCommon->selected_box = -1;

    // loop through all scene objects and determine the object clicked by
    // looking at the nearest distance to the object
    for (i = 0; i < 3; i++) {
      float dist = glm::distance(g_pCommon->box_positions[i], objPt);

      if (dist < 1 && dist < minDist) {
        g_pCommon->selected_box = static_cast<int>(i);
        minDist = dist;
      }
    }
  }

  if (button == GLUT_MIDDLE_BUTTON) {
    g_pCommon->state = 0;
  } else {
    g_pCommon->state = 1;
  }
}

// mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->selected_box == -1) {
    if (g_pCommon->state == 0) {
      g_pCommon->fov += (y - g_pCommon->oldY) / 5.0f;
      g_pCommon->cam.SetupProjection(g_pCommon->fov, g_pCommon->cam.GetAspectRatio());
    } else {
      g_pCommon->rY += (y - g_pCommon->oldY) / 5.0f;
      g_pCommon->rX += (g_pCommon->oldX - x) / 5.0f;

      if (g_pCommon->useFiltering) {
        filterMouseMoves(g_pCommon->rX, g_pCommon->rY);
      } else {
        g_pCommon->mouseX = g_pCommon->rX;
        g_pCommon->mouseY = g_pCommon->rY;
      }
      g_pCommon->cam.Rotate(g_pCommon->mouseX, g_pCommon->mouseY, 0);
    }
    g_pCommon->oldX = x;
    g_pCommon->oldY = y;
  }
  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  GL_CHECK_ERRORS

  // create a grid of size 20x20 in XZ plane
  g_pCommon->grid = new CGrid(20, 20);

  // create a unit cube
  g_pCommon->cube = new CUnitCube();

  GL_CHECK_ERRORS

  // set the camera position
  glm::vec3 p = glm::vec3(10, 10, 10);
  g_pCommon->cam.SetPosition(p);

  // get the camera look direction to obtain the yaw and pitch values for camera
  // rotation
  glm::vec3 look    = glm::normalize(p);
  const float yaw   = glm::degrees(atan2f(look.z, look.x) + static_cast<float>(M_PI));
  const float pitch = glm::degrees(asinf(look.y));
  g_pCommon->rX     = yaw;
  g_pCommon->rY     = pitch;

  // if filtering is enabled, save positions to mouse history buffer
  if (g_pCommon->useFiltering) {
    for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
      g_pCommon->mouseHistory[i] = glm::vec2(g_pCommon->rX, g_pCommon->rY);
    }
  }
  g_pCommon->cam.Rotate(g_pCommon->rX, g_pCommon->rY, 0);

  // enable depth testing
  glEnable(GL_DEPTH_TEST);

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  delete g_pCommon->grid;
  delete g_pCommon->cube;
  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // set the camera projection
  g_pCommon->cam.SetupProjection(g_pCommon->fov, static_cast<GLfloat>(w) / h);
}

// set the idle callback
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

  // clear colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the mesage
  g_pCommon->msg.str(std::string());
  if (g_pCommon->selected_box == -1) {
    g_pCommon->msg << "No box picked";
  } else {
    g_pCommon->msg << "Picked box: " << g_pCommon->selected_box;
  }

  // set the window title
  glutSetWindowTitle(g_pCommon->msg.str().c_str());

  // from the camera modelview and projection matrices get the combined MVP
  // matrix
  g_pCommon->MV = g_pCommon->cam.GetViewMatrix();
  g_pCommon->P = g_pCommon->cam.GetProjectionMatrix();
  const auto MVP = g_pCommon->P * g_pCommon->MV;

  // render the grid object
  g_pCommon->grid->Render(glm::value_ptr(MVP));

  // set the first cube transform
  // set its colour to cyan if selected, red otherwise
  auto T = glm::translate(glm::mat4(1), g_pCommon->box_positions[0]);
  g_pCommon->cube->color = (g_pCommon->selected_box == 0) ? glm::vec3(0, 1, 1) : glm::vec3(1, 0, 0);
  g_pCommon->cube->Render(glm::value_ptr(MVP * T));

  // set the second cube transform
  // set its colour to cyan if selected, green otherwise
  T = glm::translate(glm::mat4(1), g_pCommon->box_positions[1]);
  g_pCommon->cube->color = (g_pCommon->selected_box == 1) ? glm::vec3(0, 1, 1) : glm::vec3(0, 1, 0);
  g_pCommon->cube->Render(glm::value_ptr(MVP * T));

  // set the third cube transform
  // set its colour to cyan if selected, blue otherwise
  T = glm::translate(glm::mat4(1), g_pCommon->box_positions[2]);
  g_pCommon->cube->color = (g_pCommon->selected_box == 2) ? glm::vec3(0, 1, 1) : glm::vec3(0, 0, 1);
  g_pCommon->cube->Render(glm::value_ptr(MVP * T));

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  // handle the WSAD, QZ key events to move the camera around
  switch (key) {
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
  glutCreateWindow("Picking using depth buffer - OpenGL 3.3");

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
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION)              << std::endl;
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)                   << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)                 << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)                  << std::endl;
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
  glutKeyboardFunc(OnKey);
  glutIdleFunc(OnIdle);

  // call main loop
  glutMainLoop();

  return 0;
}
