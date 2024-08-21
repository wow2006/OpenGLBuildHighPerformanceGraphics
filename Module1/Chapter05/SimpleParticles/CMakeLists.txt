# ${CMAKE_SOURCE_DIR}/Module1/Chapter05/SimpleParticles/CMakeLists.txt
add_executable(
  SimpleParticles
  main.cpp
)

target_link_libraries(
  SimpleParticles
  PUBLIC
  Common
)

file(
  COPY
  ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
  DESTINATION "${OUTPUT_DIR}"
)