set(exec_name SimpleParticles)

add_executable(
  ${exec_name}
  main.cpp
)

target_link_libraries(
  ${exec_name}
  PUBLIC
  Common
)

add_custom_command(TARGET ${exec_name} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${exec_name} ${PROJECT_BINARY_DIR}/bin/Module1/Chapter05/${exec_name}
  COMMAND ${CMAKE_COMMAND} -E copy_directory    ${CMAKE_CURRENT_LIST_DIR}/shaders
                                                ${PROJECT_BINARY_DIR}/bin/Module1/Chapter05/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory    ${CMAKE_CURRENT_LIST_DIR}/media
                                                ${PROJECT_BINARY_DIR}/bin/Module1/Chapter05/media
)
