add_executable(
  GettingStarted
  main.cpp
)

target_link_libraries(
  GettingStarted
  PUBLIC
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
  options::options
)