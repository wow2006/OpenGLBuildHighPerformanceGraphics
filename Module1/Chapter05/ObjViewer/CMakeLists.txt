# ${CMAKE_SOURCE_DIR}/Module1/Chapter05/ObjViewer/CMakeLists.txt
add_executable(
  ObjViewer
  main.cpp
  Obj.cpp
)

target_link_libraries(
  ObjViewer
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)