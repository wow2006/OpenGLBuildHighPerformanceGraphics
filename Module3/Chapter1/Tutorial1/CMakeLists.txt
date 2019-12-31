add_executable(
  Tutorial1
  main.cpp
)

target_link_libraries(
  Tutorial1
  PUBLIC
  OpenGL::GL
  glfw
)

