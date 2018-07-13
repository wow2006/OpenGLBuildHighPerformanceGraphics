add_executable(SubdivisionGeometryShader main.cpp GLSLShader.cpp)
target_link_libraries(SubdivisionGeometryShader
  PUBLIC
  OpenGL::OpenGL
  GLUT::GLUT
  GLEW::GLEW)
target_include_directories(SubdivisionGeometryShader
  PUBLIC
  ${GLM_INCLUDE_DIR})
target_compile_features(SubdivisionGeometryShader
  PUBLIC
  cxx_std_14)
target_compile_options(SubdivisionGeometryShader
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes -Wno-padded>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>)

add_custom_command(TARGET SubdivisionGeometryShader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           SubdivisionGeometryShader         ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/SubdivisionGeometryShader
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders)
