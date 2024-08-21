# ${CMAKE_SOURCE_DIR}/Module1/Chapter03/CMakeLists.txt
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin/Module1/Chapter03/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_subdirectory(TwirlFilter)
add_subdirectory(Skybox)
add_subdirectory(MirrorUsingFBO)
add_subdirectory(DynamicCubemap)
add_subdirectory(Convolution)
add_subdirectory(Glow)