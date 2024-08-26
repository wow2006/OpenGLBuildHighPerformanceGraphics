# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/SphericalHarmonics/CMakeLists.txt
add_executable(SphericalHarmonics main.cpp Obj.cpp)

target_link_libraries(SphericalHarmonics PUBLIC Common)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
     DESTINATION "${OUTPUT_DIR}/SphericalHarmonicsData")
