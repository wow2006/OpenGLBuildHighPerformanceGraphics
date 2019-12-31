# ${CMAKE_SOURCE_DIR}/Module3/CMakeLists.txt

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)

add_subdirectory(Chapter01)
add_subdirectory(Chapter02)

