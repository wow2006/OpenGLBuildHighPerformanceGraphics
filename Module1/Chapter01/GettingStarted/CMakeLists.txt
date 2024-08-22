# ${CMAKE_SOURCE_DIR}/Module1/Chapter01/GettingStarted/CMakeLists.txt
add_executable(GettingStarted main.cpp)

target_link_libraries(
  GettingStarted
  PUBLIC fmt::fmt
         OpenGL::GL
         glfw
         glbinding::glbinding
         options::options)
