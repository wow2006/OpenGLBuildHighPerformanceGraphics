add_executable(PickingDepthBuffer main.cpp)
target_link_libraries(PickingDepthBuffer
  PUBLIC
  Common)
add_custom_command(TARGET PickingDepthBuffer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           PickingDepthBuffer                        ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/PickingDepthBuffer
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/shaders)
