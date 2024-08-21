# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/DualDepthPeeling/CMakeLists.txt
set(exec_name DualDepthPeeling)

add_executable(
  DualDepthPeeling
  main.cpp
)

target_link_libraries(
  DualDepthPeeling
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}/DualDepthPeeling"
)