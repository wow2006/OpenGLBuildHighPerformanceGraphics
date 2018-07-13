add_executable(RippleDeformer main.cpp GLSLShader.cpp)
target_link_libraries(RippleDeformer
  PUBLIC
  OpenGL::OpenGL
  GLUT::GLUT
  GLEW::GLEW)
target_include_directories(RippleDeformer
  PUBLIC
  ${GLM_INCLUDE_DIR})
target_compile_features(RippleDeformer
  PUBLIC
  cxx_std_14)
target_compile_options(RippleDeformer
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes -Wno-padded>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>)

add_custom_command(TARGET RippleDeformer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           RippleDeformer                    ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/RippleDeformer
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders)
