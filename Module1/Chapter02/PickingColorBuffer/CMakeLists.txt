# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/PickingColorBuffer/CMakeLists.txt
add_executable(PickingColorBuffer main.cpp)

target_link_libraries(
  PickingColorBuffer
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)