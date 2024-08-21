# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/ShadowMapping/CMakeLists.txt
add_executable(
  ShadowMapping
  main.cpp
)

target_link_libraries(
  ShadowMapping
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)