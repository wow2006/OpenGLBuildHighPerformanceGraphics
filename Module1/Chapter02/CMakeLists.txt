# ${CMAKE_SOURCE_DIR}/Module1/Chapter02/CMakeLists.txt
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin/Module1/Chapter02/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_subdirectory(FreeCamera)
add_subdirectory(TargetCamera)
add_subdirectory(ViewFrustumCulling)
add_subdirectory(PickingDepthBuffer)
add_subdirectory(PickingColorBuffer)
add_subdirectory(PickingSceneIntersection)
