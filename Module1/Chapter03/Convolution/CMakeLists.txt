add_executable(Convolution main.cpp)
target_link_libraries(Convolution
  PUBLIC
  Common)
add_custom_command(TARGET Convolution POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           Convolution                       ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/Convolution
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media   ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/media)
