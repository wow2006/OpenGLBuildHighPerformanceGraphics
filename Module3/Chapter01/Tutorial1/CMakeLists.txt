# ${CMAKE_SOURCE_DIR}/Module3/Chapter01/Tutorial1/CMakeLists.txt

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

