# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/ShadowMappingPCF/CMakeLists.txt
add_executable(ShadowMappingPCF main.cpp)

target_link_libraries(ShadowMappingPCF PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
