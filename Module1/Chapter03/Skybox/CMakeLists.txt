add_executable(SkyBox main.cpp)
target_link_libraries(SkyBox
  PUBLIC
  Common)
add_custom_command(TARGET SkyBox POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           SkyBox                            ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/SkyBox
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media   ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/media
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/shaders)

