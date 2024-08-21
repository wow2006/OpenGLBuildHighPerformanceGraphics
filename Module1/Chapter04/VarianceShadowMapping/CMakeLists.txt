# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/VarianceShadowMapping/CMakeLists.txt
set(exec_name VarianceShadowMapping)

add_executable(
  VarianceShadowMapping
  main.cpp
)

target_link_libraries(
  VarianceShadowMapping
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)