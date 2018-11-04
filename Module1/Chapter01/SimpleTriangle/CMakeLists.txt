add_executable(
  SimpleTriangle
  main.cpp
)

target_link_libraries(SimpleTriangle
  PUBLIC
  OpenGL::GL
  GLEW::GLEW
  Common
)

add_custom_command(TARGET SimpleTriangle POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           SimpleTriangle                    ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/SimpleTriangle
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders)
