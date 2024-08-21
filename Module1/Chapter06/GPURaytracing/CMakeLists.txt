# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/GPURaytracing/CMakeLists.txt
add_executable(
  GPURaytracing
  main.cpp
  Obj.cpp
)

target_link_libraries(
  GPURaytracing
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}/GPURaytracing"
)