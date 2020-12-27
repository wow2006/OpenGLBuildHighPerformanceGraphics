add_executable(
  ImageLoader
  main.cpp
  GLSLShader.cpp
)

target_link_libraries(
  ImageLoader
  PUBLIC
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
  options::options
  ${SOIL_LIBRARIES}
)

target_include_directories(
  ImageLoader
  PUBLIC
  ${GLM_INCLUDE_DIR}
  ${SOIL_INCLUDE_DIRS}
)

add_custom_command(
  TARGET ImageLoader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           ImageLoader                       ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/ImageLoader
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media   ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/media
)
