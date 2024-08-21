# ${CMAKE_SOURCE_DIR}/Module1/Chapter05/CMakeLists.txt
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin/Module1/Chapter05/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_subdirectory(TerrainLoading)
add_subdirectory(3dsViewer)
add_subdirectory(ObjViewer)
if(WIN32)
  # add_subdirectory(EZMeshViewer) FIXME(Hussein): Windows wstr problem
endif()
add_subdirectory(SimpleParticles)