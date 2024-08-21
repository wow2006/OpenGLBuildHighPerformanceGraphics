# ${CMAKE_SOURCE_DIR}/Module1/Chapter05/3dsViewer/CMakeLists.txt
add_executable(
  3dsViewer
  main.cpp
  3ds.cpp
)

target_link_libraries(
  3dsViewer
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}"
)