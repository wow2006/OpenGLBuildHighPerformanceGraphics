# ${CMAKE_SOURCE_DIR}/Module1/Chapter01/CMakeLists.txt
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_library(GLSLShader STATIC GLSLShader.cpp GLSLShader.hpp)
target_include_directories(GLSLShader PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(GLSLShader PRIVATE glbinding::glbinding)

add_subdirectory(GettingStarted)
add_subdirectory(SimpleTriangle)
add_subdirectory(ImageLoader)
add_subdirectory(RippleDeformer)
add_subdirectory(SubdivisionGeometryShader)
