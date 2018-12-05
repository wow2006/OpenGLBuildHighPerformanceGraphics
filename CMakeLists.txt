project(HighPerformanceGraphics CXX)
cmake_minimum_required(VERSION 3.9)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(GSL  REQUIRED)
find_package(GL3W REQUIRED)

add_subdirectory(Module1)
