add_executable(
  SubdivisionGeometryShader
  main.cpp
  GLSLShader.cpp
)

target_link_libraries(
  SubdivisionGeometryShader
  PUBLIC
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
  options::options
)

target_include_directories(
  SubdivisionGeometryShader
  PUBLIC
  ${GLM_INCLUDE_DIR}
)

add_custom_command(
  TARGET SubdivisionGeometryShader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           SubdivisionGeometryShader         ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/SubdivisionGeometryShader
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders
)

