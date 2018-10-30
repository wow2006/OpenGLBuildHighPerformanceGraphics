add_library(
  Common
  STATIC
  common.cpp
)

target_include_directories(
  Common
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  Common
  PUBLIC
  GSL
  SDL2::SDL2
  SDL2::SDL2main
)

target_compile_features(
  Common
  PUBLIC
  cxx_std_17
)

target_compile_options(
  Common
  PUBLIC
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes -Wno-weak-vtables>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
