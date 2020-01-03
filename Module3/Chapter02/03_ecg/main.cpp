// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// STL
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iterator>
// OpenGL
#include <GLFW/glfw3.h>

extern float gECG_Data[];

constexpr auto g_cWindowsWidth      = 640 * 2;
constexpr auto g_cWindowsHeight     = 480;
constexpr auto g_cWindowTitle       = "Chapter 2: Primitive drawings";
constexpr auto g_cEcgDataBufferSize = 1024;

struct Vertex {
  GLfloat x, y, z;
  GLfloat r, g, b, a;
};

struct Data {
  GLfloat x, y, z;
};

static float gRatio = 0;

void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);

void ecg_demo(int counter);

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

  auto counter = 0U;
  constexpr auto incrementCount = 5U;
  constexpr auto maxCounter = 5000U;
  while (!glfwWindowShouldClose(pWindow)) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);

    gRatio = static_cast<float>(width) / static_cast<float>(height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-gRatio, gRatio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawGrid(5.F, 1.F, 0.1F);

    if (counter > maxCounter) {
      counter = 0;
    }
    counter += incrementCount;

    // run the demo visualizer
    ecg_demo(counter);

    glfwSwapBuffers(pWindow);
    glfwPollEvents();
  }

  glfwDestroyWindow(pWindow);
  glfwTerminate();

  return EXIT_SUCCESS;
}

void drawLineSegment(Vertex v1, Vertex v2, GLfloat width = 1.F) {
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

void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width) {
  for (float i = -height; i < height; i += grid_width) {
    Vertex v1 = {-width, i, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    Vertex v2 = {width, i, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    drawLineSegment(v1, v2);
  }

  for (float i = -width; i < width; i += grid_width) {
    Vertex v1 = {i, -height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    Vertex v2 = {i, height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    drawLineSegment(v1, v2);
  }
}

void drawPoint(Vertex v1, GLfloat point_size) {
  glPointSize(point_size);
  glBegin(GL_POINTS);
  {
    glColor4f(v1.r, v1.g, v1.b, v1.a);
    glVertex3f(v1.x, v1.y, v1.z);
  }
  glEnd();
}

void draw2DScatterPlot(const std::vector<Data> &vPoints) {
  Vertex v1 = {-10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  Vertex v2 = {10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  drawLineSegment(v1, v2, 2.0f);
  v1.x = 0.0f;
  v2.x = 0.0f;
  v1.y = -1.0f;
  v2.y = 1.0f;
  drawLineSegment(v1, v2, 2.0f);

  for (const auto& point : vPoints) {
    GLfloat x = point.x;
    GLfloat y = point.y;
    Vertex v = {x, y, 0.0f, 1.0, 1.0, 1.0, 0.7f};
    drawPoint(v, 10.0f);
  }
}

void draw2DLineSegments(const std::vector<Data> &vPoints) {
  Vertex v1 = {-10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f};
  Vertex v2 = {10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f};
  drawLineSegment(v1, v2, 4.0f);
  v1.x = 0.0f;
  v2.x = 0.0f;
  v1.y =-1.0f;
  v2.y = 1.0f;
  drawLineSegment(v1, v2, 4.0f);

  auto iter1 = vPoints.cbegin();
  auto iter2 = iter1 + 1;
  for (; iter2 != vPoints.cend(); ++iter1, ++iter2) {
    GLfloat x1 = iter1->x;
    GLfloat y1 = iter1->y;
    GLfloat x2 = iter2->x;
    GLfloat y2 = iter2->y;

    Vertex v1 = {x1, y1, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f};
    Vertex v2 = {x2, y2, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f};
    drawLineSegment(v1, v2, 4.0f);
  }
}

void plotECGData(int offset, int size, float offset_y, float scale) {
  const float space = 2.0f / size * gRatio;

  float pos = -size * space / 2.0f;

  glLineWidth(5.0f);

  glBegin(GL_LINE_STRIP);

  glColor4f(0.1f, 1.0f, 0.1f, 0.8f);

  for (int i = offset; i < size + offset; i++) {
    const float data = scale * gECG_Data[i] + offset_y;
    glVertex3f(pos, data, 0.0f);
    pos += space;
  }

  glEnd();
}

void ecg_demo(int counter) {
  const int dataSize = g_cEcgDataBufferSize;

  plotECGData(counter,                dataSize,-0.5f, 0.1f);
  plotECGData(counter + dataSize,     dataSize, 0.0f, 0.5f);
  plotECGData(counter + dataSize * 2, dataSize, 0.5f,-0.25f);
}
