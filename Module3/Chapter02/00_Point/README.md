## Tutorial 1:

![Point](Screenshot.gif)

Draw single point on fixed pipeline OpenGL.

#### Contents:
1. Initialize GLFW3.
`glfwInit`
2. Create GLFW3 Window.
`glfwCreateWindow`
3. Set Window to OpenGL Context.
`glfwMakeContextCurrent`
4. Set Key callback function.
`glfwSetKeyCallback`
4. Game loop.
  - Check for GLFW closed.
  - Toggle smooth point.
  - Get Framebuffer size.
  - Set Viewport.
  - Clear Framebuffer.
  - Set Projection Matrix.
  - Set ModelView Matrix.
  - Draw points.
  - Swap buffer.
  - Process events.

