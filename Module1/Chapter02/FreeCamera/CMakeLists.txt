# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/FreeCamera/CMakeLists.txt
add_executable(
  FreeCamera
  main.cpp
)

target_link_libraries(
  FreeCamera
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)