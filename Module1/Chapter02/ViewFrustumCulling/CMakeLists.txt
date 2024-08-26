# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/ViewFrustumCulling/CMakeLists.txt
add_executable(ViewFrustumCulling main.cpp)

target_link_libraries(ViewFrustumCulling PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
