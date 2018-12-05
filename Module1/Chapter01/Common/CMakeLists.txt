project(Common CXX)

add_library(
  ${PROJECT_NAME}
  STATIC
  common.cpp
  ShaderLoader.cpp
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
  GSL
  gl3w::gl3w
  OpenGL::GL
  SDL2::SDL2
  SDL2::SDL2main
  PRIVATE
  ${CMAKE_DL_LIBS}
)

target_compile_features(
  ${PROJECT_NAME}
  PUBLIC
  cxx_std_17
)

target_compile_options(
  ${PROJECT_NAME}
  PUBLIC
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes -Wno-weak-vtables -Wno-padded -Wno-reserved-id-macro>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
