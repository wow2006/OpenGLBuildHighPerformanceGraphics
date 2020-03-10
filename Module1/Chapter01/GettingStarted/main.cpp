// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// STL
#include <iostream>
// GLFW
#include <GLFW/glfw3.h>

// screen size
const int WIDTH = 1280;
const int HEIGHT = 960;

int main(int argc, char **argv) {
  if (!glfwInit()) {
    return EXIT_FAILURE;
  }

  GLFWwindow *pWindow =
      glfwCreateWindow(WIDTH, HEIGHT, "GettingStarted", nullptr, nullptr);
  if (pWindow == nullptr) {
    glfwTerminate();
    return EXIT_FAILURE;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(pWindow);

  glClearColor(1, 0, 0, 0);
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(pWindow)) {
    // clear colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Swap front and back buffers */
    glfwSwapBuffers(pWindow);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return EXIT_SUCCESS;
}
