add_executable(PickingColorBuffer main.cpp)
target_link_libraries(PickingColorBuffer
  PUBLIC
  Common)
add_custom_command(TARGET PickingColorBuffer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           PickingColorBuffer                        ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/PickingColorBuffer
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/shaders)
