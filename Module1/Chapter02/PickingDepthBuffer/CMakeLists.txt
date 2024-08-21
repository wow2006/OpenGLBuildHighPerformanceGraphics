# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/PickingDepthBuffer/CMakeLists.txt
add_executable(PickingDepthBuffer main.cpp)

target_link_libraries(
  PickingDepthBuffer
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)