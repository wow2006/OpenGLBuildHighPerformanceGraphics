# ${CMAKE_SOURCE_DIR}/Module3/Chapter02/02_sinusoid/CMakeLists.txt

add_executable(
  02_sinusoid
  main.cpp
)

target_link_libraries(
  02_sinusoid
  PUBLIC
  OpenGL::GL
  glfw
)

