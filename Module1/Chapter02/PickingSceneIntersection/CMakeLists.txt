# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/PickingSceneIntersection/CMakeLists.txt
add_executable(PickingSceneIntersection main.cpp)

target_link_libraries(PickingSceneIntersection PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION "${OUTPUT_DIR}")
