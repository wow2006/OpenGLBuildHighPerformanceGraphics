# ${CMAKE_SOURCE_DIR}/Module1/Chapter08/TransformFeedbackParticles/CMakeLists.txt
add_executable(
  TransformFeedbackParticles
  main.cpp
)

target_link_libraries(
  TransformFeedbackParticles
  PRIVATE
  Common
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
)

target_compile_features(
  TransformFeedbackParticles
  PRIVATE
  cxx_std_17
)

target_compile_options(
  TransformFeedbackParticles
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall;-Weverything;-Wno-c++98-compat;-Wno-c++98-compat-pedantic;-Werror;-Wno-missing-prototypes;-Wno-padded>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic;-Wall;-Wextra;-Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
