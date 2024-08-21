# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/TwirlFilter/CMakeLists.txt
add_executable(TwirlFilter main.cpp)

target_link_libraries(
  TwirlFilter
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}"
)