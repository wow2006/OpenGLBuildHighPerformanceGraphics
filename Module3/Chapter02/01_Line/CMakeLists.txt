# ${CMAKE_SOURCE_DIR}/Module3/Chapter02/01_Line/CMakeLists.txt

add_executable(
  01_Line
  main.cpp
)

target_link_libraries(
  01_Line
  PUBLIC
  OpenGL::GL
  glfw
)

