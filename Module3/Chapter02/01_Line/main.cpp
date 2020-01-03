// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// STL
#include <cstdlib>
#include <string_view>
// OpenGL
#include <GLFW/glfw3.h>

constexpr auto g_cWindowsWidth = 640 * 2;
constexpr auto g_cWindowsHeight = 480;
constexpr auto g_cWindowTitle = "Chapter 2: Primitive drawings";

struct Vertex {
  GLfloat x, y, z;
  GLfloat r, g, b, a;
};

void drawLineSegment(Vertex v1, Vertex v2, GLfloat width = 1.0f);

void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);

void drawLineDemo();

auto main() -> int {
  if (!glfwInit()) {
    return EXIT_FAILURE;
  }

  GLFWwindow *pWindow = glfwCreateWindow(g_cWindowsWidth, g_cWindowsHeight,
                                         g_cWindowTitle, nullptr, nullptr);
  if (pWindow == nullptr) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(pWindow);

  // enable anti-aliasing
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POINT_SMOOTH);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(pWindow)) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);

    const float ratio = static_cast<float>(width) / static_cast<float>(height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawLineDemo();

    glfwSwapBuffers(pWindow);
    glfwPollEvents();
  }

  glfwDestroyWindow(pWindow);
  glfwTerminate();
  return EXIT_SUCCESS;
}

void drawLineDemo() {
  // Draw a simple grid
  drawGrid(5.0F, 1.0F, 0.1F);

  Vertex v1 = {-5.F, 0.F, 0.F, 1.F, 0.F, 0.F, 0.7F};
  Vertex v2 = { 5.F, 0.F, 0.F, 0.F, 1.F, 0.F, 0.7F};
  Vertex v3 = { 0.F, 1.F, 0.F, 0.F, 0.F, 1.F, 0.7F};
  Vertex v4 = { 0.F,-1.F, 0.F, 0.F, 0.F, 1.F, 0.7F};

  drawLineSegment(v1, v2, 10.F);
  drawLineSegment(v3, v4, 10.F);
}

void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width) {
  // horizontal lines
  for (float i = -height; i < height; i += grid_width) {
    Vertex v1 = {-width, i, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    Vertex v2 = {width, i, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    drawLineSegment(v1, v2);
  }
  // vertical lines
  for (float i = -width; i < width; i += grid_width) {
    Vertex v1 = {i, -height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    Vertex v2 = {i, height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    drawLineSegment(v1, v2);
  }
}

void drawLineSegment(Vertex v1, Vertex v2, GLfloat width) {
  glLineWidth(width);

  glBegin(GL_LINES);
  {
    glColor4f(v1.r, v1.g, v1.b, v1.a);
    glVertex3f(v1.x, v1.y, v1.z);

    glColor4f(v2.r, v2.g, v2.b, v2.a);
    glVertex3f(v2.x, v2.y, v2.z);
  }
  glEnd();
}
