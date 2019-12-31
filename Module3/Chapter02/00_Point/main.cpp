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

void drawPointsDemo();

void drawPoint(Vertex v1, GLfloat point_size);

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

    drawPointsDemo();

    glfwSwapBuffers(pWindow);
    glfwPollEvents();
  }

  glfwDestroyWindow(pWindow);
  glfwTerminate();
  return EXIT_SUCCESS;
}

void drawPointsDemo() {
  for (GLfloat x = 0.0f, size = 5.0f; x <= 1.0f; x += 0.2f, size += 5.F) {
    Vertex v1 = {x, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    drawPoint(v1, size);
  }
}

void drawPoint(Vertex v1, GLfloat point_size) {
  // draw a point and define the size, color, and location
  glPointSize(point_size);
  glBegin(GL_POINTS);
  {
    glColor4f(v1.r, v1.g, v1.b, v1.a);
    glVertex3f(v1.x, v1.y, v1.z);
  }
  glEnd();
}
