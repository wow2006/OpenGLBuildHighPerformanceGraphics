// STL
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <vector>
// OpenGL
#include <GLFW/glfw3.h>

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

void drawGrid(GLfloat width, GLfloat height, GLfloat grid_width);

void gaussianDemo(float sigma);

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

  auto sigma = 0.01F;

  while (!glfwWindowShouldClose(pWindow)) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);

    const auto ratio = static_cast<float>(width) / static_cast<float>(height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // simple grid
    drawGrid(5.0f, 1.0f, 0.1f);

    // draw the 2D Gaussian function with a heatmap
    sigma += 0.01f;
    if (sigma > 1.0f) {
      sigma = 0.01;
    }
    gaussianDemo(sigma);

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

  for (const auto &point : vPoints) {
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
  v1.y = -1.0f;
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

void draw2DHeatMap(const Data *data, int num_points) {
  // locate the maximum and minimum values in the dataset
  float max_value = -999.9f;
  float min_value = 999.9f;
  for (int i = 0; i < num_points; i++) {
    const Data d = data[i];
    if (d.z > max_value) {
      max_value = d.z;
    }
    if (d.z < min_value) {
      min_value = d.z;
    }
  }
  const float halfmax = (max_value + min_value) / 2;

  // display the result
  glPointSize(2.0f);
  glBegin(GL_POINTS);

  for (int i = 0; i < num_points; i++) {
    const Data d = data[i];
    float value = d.z;
    float b = 1.0f - value / halfmax;
    float r = value / halfmax - 1.0f;
    if (b < 0) {
      b = 0;
    }
    if (r < 0) {
      r = 0;
    }
    float g = 1.0f - b - r;

    glColor4f(r, g, b, 0.5f);
    glVertex3f(d.x, d.y, 0.0f);
  }
  glEnd();
}

void gaussianDemo(float sigma) {
  // construct a 1000x1000 grid
  const int grid_x = 1000;
  const int grid_y = 1000;
  const int num_points = grid_x * grid_y;
  Data *data = (Data *)malloc(sizeof(Data) * num_points);
  int data_counter = 0;
  for (int x = -grid_x / 2; x < grid_x / 2; x += 1) {
    for (int y = -grid_y / 2; y < grid_y / 2; y += 1) {
      float x_data = 2.0f * x / grid_x;
      float y_data = 2.0f * y / grid_y;
      // compute the height z based on a 2-D Gaussian function.
      float z_data = exp(-0.5f * (x_data * x_data) / (sigma * sigma) -
                         0.5f * (y_data * y_data) / (sigma * sigma)) /
                     (sigma * sigma * 2.0f * M_PI);
      data[data_counter].x = x_data;
      data[data_counter].y = y_data;
      data[data_counter].z = z_data;
      data_counter++;
    }
  }
  // visualize the result using a 2D heat map
  draw2DHeatMap(data, num_points);
  free(data);
}
