# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/TargetCamera/CMakeLists.txt
add_executable(TargetCamera main.cpp)

target_link_libraries(TargetCamera PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
