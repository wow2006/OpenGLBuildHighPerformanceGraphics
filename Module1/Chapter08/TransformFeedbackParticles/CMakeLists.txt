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
  options::options
)

