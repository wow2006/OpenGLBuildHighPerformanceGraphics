# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/PerFragmentLighting/CMakeLists.txt
add_executable(
  PerFragmentLighting
  main.cpp
)

target_link_libraries(
  PerFragmentLighting
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)