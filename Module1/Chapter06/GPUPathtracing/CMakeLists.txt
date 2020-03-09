# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/GPUPathtracing/CMakeLists.txt
set(exec_name GPUPathtracing)

add_executable(
  ${exec_name}
  main.cpp
  Obj.cpp
)

target_link_libraries(
  ${exec_name}
  PUBLIC
  Common
)

add_custom_command(
  TARGET ${exec_name}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           ${exec_name}                      ${PROJECT_BINARY_DIR}/bin/Module1/Chapter06/${exec_name}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter06/shaders
)

