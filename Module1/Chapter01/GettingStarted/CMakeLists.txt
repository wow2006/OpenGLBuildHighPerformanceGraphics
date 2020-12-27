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

add_custom_command(
  TARGET GettingStarted POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy GettingStarted
  ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/GettingStarted
)

