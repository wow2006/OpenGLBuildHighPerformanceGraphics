add_executable(MirrorFBO main.cpp)
target_link_libraries(MirrorFBO
  PUBLIC
  Common)
add_custom_command(TARGET MirrorFBO POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           MirrorFBO                         ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/MirrorFBO
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/shaders)

