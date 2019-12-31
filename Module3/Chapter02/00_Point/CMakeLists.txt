# ${CMAKE_SOURCE_DIR}/Module3/Chapter02/00_Point/CMakeLists.txt

add_executable(
  00_Point
  main.cpp
)

target_link_libraries(
  00_Point
  PUBLIC
  OpenGL::GL
  glfw
)

