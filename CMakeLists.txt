cmake_minimum_required(VERSION 3.26)
project(HighPerformanceGraphics CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(options)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(fmt CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Stb REQUIRED)

add_subdirectory(Module1)
add_subdirectory(Module3)