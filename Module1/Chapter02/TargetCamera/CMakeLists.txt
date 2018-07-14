add_executable(TargetCamera main.cpp)
target_link_libraries(TargetCamera
  PUBLIC
  Common)
add_custom_command(TARGET TargetCamera POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           TargetCamera                      ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/TargetCamera
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/shaders)
