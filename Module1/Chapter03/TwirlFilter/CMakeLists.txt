add_executable(TwirlFilter main.cpp)
target_link_libraries(TwirlFilter
  PUBLIC
  Common)
add_custom_command(TARGET TwirlFilter POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           TwirlFilter                       ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/TwirlFilter
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media   ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/media)
