# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/Glow/CMakeLists.txt
add_executable(Glow main.cpp)

target_link_libraries(
  Glow
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}"
)