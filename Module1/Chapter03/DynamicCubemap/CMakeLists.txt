# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/DynamicCubemap/CMakeLists.txt
add_executable(DynamicCubemap main.cpp)

target_link_libraries(
  DynamicCubemap
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)