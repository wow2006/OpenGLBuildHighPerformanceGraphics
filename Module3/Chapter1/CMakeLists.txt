# ${CMAKE_SOURCE_DIR}/Module3/Chapter1/CMakeLists.txt

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)

add_subdirectory(Tutorial1)

