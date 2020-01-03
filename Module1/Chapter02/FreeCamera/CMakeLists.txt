# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/FreeCamera/CMakeLists.txt

add_executable(
  FreeCamera
  main.cpp
)

target_link_libraries(
  FreeCamera
  PUBLIC
  Common
)

add_custom_command(
  TARGET FreeCamera
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           FreeCamera                        ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/FreeCamera
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/shaders
)

