add_executable(Glow main.cpp)
target_link_libraries(Glow
  PUBLIC
  Common)
add_custom_command(TARGET Glow POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           Glow                    ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/Glow
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/shaders)

