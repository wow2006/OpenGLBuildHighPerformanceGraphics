# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/Convolution/CMakeLists.txt
add_executable(Convolution main.cpp)

target_link_libraries(Convolution PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media DESTINATION "${OUTPUT_DIR}")
