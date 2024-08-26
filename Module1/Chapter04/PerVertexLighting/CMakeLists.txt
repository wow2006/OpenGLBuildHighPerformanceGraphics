# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/PerVertexLighting/CMakeLists.txt
add_executable(PerVertexLighting main.cpp)

target_link_libraries(PerVertexLighting PUBLIC imgui::imgui glfw Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
