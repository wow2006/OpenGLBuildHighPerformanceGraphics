add_executable(DynamicCubemap main.cpp)
target_link_libraries(DynamicCubemap
  PUBLIC
  Common)
add_custom_command(TARGET DynamicCubemap POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           DynamicCubemap                    ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/DynamicCubemap
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/shaders)

