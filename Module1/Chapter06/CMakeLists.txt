# ${CMAKE_SOURCE_DIR}/Module1/Chapter06/CMakeLists.txt
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin/Module1/Chapter06/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_subdirectory(FrontToBackPeeling)
add_subdirectory(DualDepthPeeling)
add_subdirectory(SSAO)
add_subdirectory(SphericalHarmonics)
add_subdirectory(GPURaytracing)
add_subdirectory(GPUPathtracing)