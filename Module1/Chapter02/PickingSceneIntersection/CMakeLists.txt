add_executable(PickingSceneIntersection main.cpp)
target_link_libraries(PickingSceneIntersection
  PUBLIC
  Common)
add_custom_command(TARGET PickingSceneIntersection POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           PickingSceneIntersection                        ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/PickingSceneIntersection
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/shaders)
