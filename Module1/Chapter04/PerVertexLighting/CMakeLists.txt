# ${}
find_package(Imgui REQUIRED)

add_executable(
  PerVertexLighting
  main.cpp
)

target_link_libraries(
  PerVertexLighting
  PUBLIC
  Imgui::GLFW_OpenGL
  Common
)

add_custom_command(
  TARGET PerVertexLighting POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy PerVertexLighting ${PROJECT_BINARY_DIR}/bin/Module1/Chapter04/PerVertexLighting
  COMMAND ${CMAKE_COMMAND} -E copy_directory         ${CMAKE_CURRENT_LIST_DIR}/shaders
                                                     ${PROJECT_BINARY_DIR}/bin/Module1/Chapter04/shaders
)
