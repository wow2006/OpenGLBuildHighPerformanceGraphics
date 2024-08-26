add_executable(RippleDeformer main.cpp GLSLShader.cpp)

target_link_libraries(
  RippleDeformer
  PUBLIC OpenGL::GL
         GLUT::GLUT
         GLEW::GLEW
         options::options)

target_include_directories(RippleDeformer PUBLIC ${GLM_INCLUDE_DIR})

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/)
