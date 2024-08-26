# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/MirrorFBO/CMakeLists.txt
add_executable(MirrorFBO main.cpp)

target_link_libraries(MirrorFBO PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
