# ${CMAKE_SOURCE_DIR}/Module1/Chapter05/TerrainLoading/CMakeLists.txt
add_executable(
  TerrainLoading
  main.cpp
)

target_link_libraries(
  TerrainLoading
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}"
)