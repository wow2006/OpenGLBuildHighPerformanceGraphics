add_executable(
  RippleDeformer
  main.cpp
  GLSLShader.cpp
)

target_link_libraries(
  RippleDeformer
  PUBLIC
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
  options::options
)

target_include_directories(
  RippleDeformer
  PUBLIC
  ${GLM_INCLUDE_DIR}
)

add_custom_command(
  TARGET RippleDeformer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           RippleDeformer                    ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/RippleDeformer
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders
)

