add_executable(ImageLoader main.cpp GLSLShader.cpp)
target_link_libraries(ImageLoader
  PUBLIC
  OpenGL::OpenGL
  GLUT::GLUT
  GLEW::GLEW
  ${SOIL_LIBRARIES})
target_include_directories(ImageLoader
  PUBLIC
  ${GLM_INCLUDE_DIR}
  ${SOIL_INCLUDE_DIRS})
target_compile_features(ImageLoader
  PUBLIC
  cxx_std_14)
target_compile_options(ImageLoader
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes -Wno-padded>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>)

add_custom_command(TARGET ImageLoader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy           ImageLoader                       ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/ImageLoader
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media   ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/media
)
