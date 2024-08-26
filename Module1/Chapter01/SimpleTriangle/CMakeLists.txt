add_executable(SimpleTriangle main.cpp GLSLShader.cpp)

target_link_libraries(
  SimpleTriangle
  PUBLIC OpenGL::GL
         GLUT::GLUT
         GLEW::GLEW
         options::options)

target_include_directories(SimpleTriangle PUBLIC ${GLM_INCLUDE_DIR})

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/)
