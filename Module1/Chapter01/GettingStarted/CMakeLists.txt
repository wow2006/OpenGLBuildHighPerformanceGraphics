# ${CMAKE_SOURCE_DIR}/Module1/Chapter01/GettingStarted/CMakeLists.txt
project(GettingStarted CXX)

add_executable(
  ${PROJECT_NAME}
  main.cpp
)

target_link_libraries(
  GettingStarted
  PUBLIC
  CONAN_PKG::glfw
  CONAN_PKG::imgui
  CONAN_PKG::glm
  options::options
)

#add_custom_command(
#  TARGET ${PROJECT_NAME}
#  POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}
#  ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/${PROJECT_NAME}
#)
