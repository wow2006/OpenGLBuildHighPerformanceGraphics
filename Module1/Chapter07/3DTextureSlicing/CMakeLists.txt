# ${CMAKE_SOURCE_DIR}/Module1/Chapter07/3DTextureSlicing/CMakeLists.txt
add_executable(
  TextureSlicing3D
  main.cpp
)

target_link_libraries(
  TextureSlicing3D
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}"
)