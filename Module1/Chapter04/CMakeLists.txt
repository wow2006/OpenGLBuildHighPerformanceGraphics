# ${CMAKE_SOURCE_DIR}/Module1/Chapter04/CMakeLists.txt
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin/Module1/Chapter04/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_subdirectory(PerVertexLighting)
add_subdirectory(PerFragmentLighting)
add_subdirectory(DirectionalLight)
add_subdirectory(ShadowMapping)
add_subdirectory(ShadowMappingPCF)
add_subdirectory(VarianceShadowMapping)
