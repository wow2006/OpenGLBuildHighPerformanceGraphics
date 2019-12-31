# ${CMAKE_SOURCE_DIR}/Module3/Chapter02/04_gaussian/CMakeLists.txt

add_executable(
  04_gaussian
  main.cpp
)

target_link_libraries(
  04_gaussian
  PUBLIC
  OpenGL::GL
  glfw
)

