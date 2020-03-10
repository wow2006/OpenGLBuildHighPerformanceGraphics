# ${CMAKE_SOURCE_DIR}/CMakeLists.txt
project(HighPerformanceGraphics CXX)
cmake_minimum_required(VERSION 3.9)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(options)

set(CONAN_EXTRA_REQUIRES
  ${CONAN_EXTRA_REQUIRES}
  glfw/3.3.2@bincrafters/stable
  imgui/1.73@bincrafters/stable
  glm/0.9.9.5@g-truc/stable
)

include(cmake/conan.cmake)
run_conan()

add_subdirectory(Module1)
#add_subdirectory(Module3)

