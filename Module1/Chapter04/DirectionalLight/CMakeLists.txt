# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/DirectionalLight/CMakeLists.txt
add_executable(DirectionalLight main.cpp)

target_link_libraries(DirectionalLight PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
