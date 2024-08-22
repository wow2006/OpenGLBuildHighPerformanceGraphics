# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/GPUPathtracing/CMakeLists.txt
set(exec_name GPUPathtracing)

add_executable(
  ${exec_name}
  main.cpp
  Obj.cpp
)

target_link_libraries(
  ${exec_name}
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}/GPUPathtracingData"
)