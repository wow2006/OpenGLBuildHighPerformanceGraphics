# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/SSAO/CMakeLists.txt
add_executable(SSAO main.cpp Obj.cpp)

target_link_libraries(SSAO PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media DESTINATION "${OUTPUT_DIR}/SSAOData")
