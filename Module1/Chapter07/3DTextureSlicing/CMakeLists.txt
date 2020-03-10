# ${CMAKE_SOURCE_DIR}/Module1/Chapter07/3DTextureSlicing/CMakeLists.txt
project(TextureSlicing3D CXX)

add_executable(
  ${PROJECT_NAME}
  main.cpp
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
  Common
)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           ${PROJECT_NAME}                   ${CMAKE_BINARY_DIR}/bin/Module1/Chapter07/${PROJECT_NAME}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_BINARY_DIR}/bin/Module1/Chapter07/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media ${CMAKE_BINARY_DIR}/bin/Module1/Chapter07/media
)

