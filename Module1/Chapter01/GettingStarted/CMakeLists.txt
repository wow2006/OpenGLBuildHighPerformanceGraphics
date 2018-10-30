set(exec_name GettingStarted)

add_executable(
  ${exec_name}
  main.cpp
)

target_link_libraries(
  ${exec_name}
  PUBLIC
  OpenGL::GL
  GLEW::GLEW
  Common
)

add_custom_command(TARGET ${exec_name} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${exec_name}
  ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/${exec_name}
)

