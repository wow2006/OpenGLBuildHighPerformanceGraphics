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
  i.    Check for GLFW closed.
  ii.   Toggle smooth point.
  iii.  Get Framebuffer size.
  iv.   Set Viewport.
  v.    Clear Framebuffer.
  vi.   Set Projection Matrix.
  vii.  Set ModelView Matrix.
  viii. Draw points.
  ix.   Swap buffer.
  x.    Process events.

