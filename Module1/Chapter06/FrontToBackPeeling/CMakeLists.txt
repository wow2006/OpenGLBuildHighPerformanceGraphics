# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/FrontToBackPeeling/CMakeLists.txt
add_executable(
  FrontToBackPeeling
  main.cpp
)

target_link_libraries(
  FrontToBackPeeling
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders
  DESTINATION "${OUTPUT_DIR}/FrontToBackPeeling"
)