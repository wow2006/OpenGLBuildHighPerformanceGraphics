# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/SkyBox/CMakeLists.txt
add_executable(SkyBox main.cpp)

target_link_libraries(SkyBox PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media DESTINATION "${OUTPUT_DIR}")
