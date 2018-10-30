add_executable(GettingStarted main.cpp)
target_link_libraries(GettingStarted
  PUBLIC
  OpenGL::GL
  SDL2::SDL2
  SDL2::SDL2main
  GLEW::GLEW)
target_compile_features(GettingStarted
  PUBLIC
  cxx_std_17)
target_compile_options(GettingStarted
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>)

add_custom_command(TARGET GettingStarted POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy GettingStarted
  ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/GettingStarted)
