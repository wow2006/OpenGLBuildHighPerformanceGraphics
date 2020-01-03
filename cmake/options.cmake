# ${CMAKE_SOURCE_DIR}/cmake/options.cmake
add_library(
  options
  INTERFACE
)

add_library(
  options::options
  ALIAS
  options
)

target_compile_features(
  options
  INTERFACE
  cxx_std_17
)

target_compile_options(
  options
  INTERFACE
  $<$<CXX_COMPILER_ID:Clang>:
  -Weverything
  #-Werror
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-missing-prototypes
  -Wno-padded
  -fcolor-diagnostics
  >
  $<$<CXX_COMPILER_ID:GNU>:
  -Wall
  -Wextra
  #-Werror
  -Wshadow
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wpedantic
  -Wconversion
  -Wmisleading-indentation
  -Wduplicated-cond
  -Wduplicated-branches
  -Wnull-dereference
  -Wuseless-cast
  -Wformat=2
  -fdiagnostics-color=always
  >
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

