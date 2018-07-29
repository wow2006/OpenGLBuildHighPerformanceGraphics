add_executable(ViewFrustumCulling main.cpp)
target_link_libraries(ViewFrustumCulling
  PUBLIC
  Common)
add_custom_command(TARGET ViewFrustumCulling POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           ViewFrustumCulling                ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/ViewFrustumCulling
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/shaders)
